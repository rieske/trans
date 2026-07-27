#include "CodeGeneratingVisitor.h"
#include "ast/InitializerListExpression.h"

#include <cassert>
#include <stdexcept>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/TypeQuery.h"

#include "quadruples/Assign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Retrieve.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Inc.h"
#include "quadruples/IndexAddress.h"
#include "quadruples/PointerDiff.h"
#include "quadruples/PointerOffset.h"
#include "quadruples/FieldAddress.h"
#include "quadruples/Dec.h"
#include "quadruples/AddressOf.h"
#include "quadruples/FunctionAddress.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/UnaryNot.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/Add.h"
#include "quadruples/Sub.h"
#include "quadruples/Mul.h"
#include "quadruples/Div.h"
#include "quadruples/Mod.h"
#include "quadruples/And.h"
#include "quadruples/Or.h"
#include "quadruples/Xor.h"
#include "quadruples/Return.h"
#include "quadruples/VoidReturn.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/EndProcedure.h"
#include "quadruples/Shl.h"
#include "quadruples/Shr.h"

namespace codegen {

CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store) : store_ { store } {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    // File-scope variables are initialized in .data; skip children (would emit assigns with no procedure).
    if (declarator.hasInitializer() && declarator.getHolder()->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!declarator.hasInitializer()) {
        return;
    }
    auto* holder = declarator.getHolder();
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        for (const auto& field : fieldStores) {
            instructions.push_back(std::make_unique<FieldAddress>(
                    holder->getName(), field.offsetBytes, field.addressName,
                    symbols::AddressBaseMode::LeaObject));
            if (field.zeroInitialize) {
                instructions.push_back(std::make_unique<AssignConstant>("0", field.sourceName));
            }
            instructions.push_back(std::make_unique<LvalueAssign>(field.sourceName, field.addressName));
        }
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        instructions.push_back(std::make_unique<Assign>(
                declarator.getInitializerHolder(store_)->getName(), holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);
    if (!arrayAccess.getLvalue(store_) || !arrayAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* indexPlan = store_.addressPlan(&arrayAccess);
    const auto* index = indexPlan ? symbols::get_if<symbols::IndexPlan>(indexPlan) : nullptr;
    // SA always publishes IndexPlan for successful array access analysis.
    assert(index && "IndexPlan required for array codegen");
    instructions.push_back(std::make_unique<IndexAddress>(
            arrayAccess.leftOperandSymbol(store_)->getName(),
            arrayAccess.rightOperandSymbol(store_)->getName(),
            arrayAccess.getElementSize(),
            arrayAccess.getLvalue(store_)->getName(),
            index->baseMode));
    if (!arrayAccess.holdsAggregateAddress()) {
        // Load scalar element for rvalue uses; stores use LvalueAssign on the address temp.
        instructions.push_back(std::make_unique<Dereference>(
                arrayAccess.getLvalue(store_)->getName(),
                arrayAccess.getLvalue(store_)->getName(),
                arrayAccess.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
}

void CodeGeneratingVisitor::visit(ast::MemberAccess& memberAccess) {
    memberAccess.getBase()->accept(*this);
    if (!memberAccess.getFieldAddressSymbol(store_) || !memberAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* plan = store_.addressPlan(&memberAccess);
    const auto* field = plan ? symbols::get_if<symbols::FieldPlan>(plan) : nullptr;
    assert(field && "FieldPlan required for member access codegen");
    const std::string addrTemp = !field->addressTempName.empty()
            ? field->addressTempName
            : memberAccess.getFieldAddressSymbol(store_)->getName();
    instructions.push_back(std::make_unique<FieldAddress>(
            memberAccess.getBase()->getResultSymbol(store_)->getName(),
            field->fieldOffsetBytes,
            addrTemp,
            field->baseMode));
    if (!memberAccess.holdsAggregateAddress()) {
        instructions.push_back(std::make_unique<Dereference>(
                addrTemp, addrTemp, memberAccess.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    for (auto& expression : functionCall.getArgumentList()) {
        instructions.push_back(std::make_unique<Argument>(expression->getResultSymbol(store_)->getName()));
    }

    const symbols::CallPlan* plan = store_.callPlan(&functionCall);
    if (!plan) {
        // SA error path — no IR.
        return;
    }
    instructions.push_back(std::make_unique<Call>(
            symbols::callCalleeName(*plan), symbols::isIndirectCall(*plan)));
    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
        instructions.push_back(std::make_unique<Retrieve>(functionCall.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    // Function designators always carry FunctionDesignatorPlan from SA (label + temp).
    if (const auto* plan = store_.addressPlan(&identifier)) {
        if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
            instructions.push_back(std::make_unique<FunctionAddress>(
                    d->functionName, d->addressTempName));
            return;
        }
    }
    assert(!identifier.holdsFunctionDesignator()
            && "designator form without FunctionDesignatorPlan on the store");
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    instructions.push_back(std::make_unique<AssignConstant>(constant.getValue(), constant.getResultSymbol(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    instructions.push_back(
        std::make_unique<AssignConstant>(stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol(store_)->getName())
    );
}

namespace {

// Scalar ++/-- steps by 1; pointer ++/-- steps by pointee size in bytes.
int incDecStepBytes(const type::Type& valueType) {
    if (valueType.isPointer()) {
        return type::pointerElementStride(valueType);
    }
    return 1;
}

} // namespace

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    auto preOperationSymbol = expression.getPreOperationSymbol()->getName();
    instructions.push_back(std::make_unique<Assign>(resultSymbolName, preOperationSymbol));

    const int step = incDecStepBytes(expression.getResultSymbol(store_)->getType());
    if (expression.getOperator()->getLexeme() == "++") {
        instructions.push_back(std::make_unique<Inc>(resultSymbolName, step));
    } else if (expression.getOperator()->getLexeme() == "--") {
        instructions.push_back(std::make_unique<Dec>(resultSymbolName, step));
    }

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
        instructions.push_back(std::make_unique<LvalueAssign>(resultSymbolName, lvalue->getName()));
    }

    expression.setResultSymbol(store_, *expression.getPreOperationSymbol());
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    const int step = incDecStepBytes(expression.getResultSymbol(store_)->getType());
    if (expression.getOperator()->getLexeme() == "++") {
        instructions.push_back(std::make_unique<Inc>(resultSymbolName, step));
    } else if (expression.getOperator()->getLexeme() == "--") {
        instructions.push_back(std::make_unique<Dec>(resultSymbolName, step));
    }

    if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
        instructions.push_back(std::make_unique<LvalueAssign>(resultSymbolName, lvalue->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    if (expression.getOperator()->getLexeme() == "sizeof") {
        // Operand is unevaluated at runtime; emit the folded size constant.
        instructions.push_back(std::make_unique<AssignConstant>(
                std::to_string(expression.getSizeofValue()), expression.getResultSymbol(store_)->getName()));
        return;
    }

    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        // &function designator: SA reuses the designator temp (already emitted FunctionAddress).
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            break;
        } else if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
            // &a[i] / &*p: address is already computed in the operand's lvalue temp.
            instructions.push_back(std::make_unique<Assign>(
                    lvalue->getName(), expression.getResultSymbol(store_)->getName()));
        } else {
            instructions.push_back(std::make_unique<AddressOf>(
                    expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '*':
        if (expression.operandSymbol(store_)->getType().isPointer()) {
            // *fp for pointer-to-function: SA keeps the pointer value (no memory load).
            if (type::isPointerToBareFunction(expression.operandSymbol(store_)->getType())) {
                if (expression.operandSymbol(store_)->getName() != expression.getResultSymbol(store_)->getName()) {
                    instructions.push_back(std::make_unique<Assign>(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
                break;
            }
            // Already an address (pointer or multi-dim decayed row).
            if (expression.getResultSymbol(store_)->getName() == expression.getLvalueSymbol(store_)->getName()) {
                // Address-only multi-dim *a: just materialize &array into the temp if needed.
                // Result and lvalue share the address temp; operand is the array object.
                if (expression.operandType().isArray()) {
                    instructions.push_back(std::make_unique<AddressOf>(
                            expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
                } else {
                    instructions.push_back(std::make_unique<Assign>(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
            } else {
                instructions.push_back(std::make_unique<Dereference>(expression.operandSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
            }
        } else if (expression.operandType().isArray()) {
            // True array object: &a then optional load.
            instructions.push_back(std::make_unique<AddressOf>(
                    expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
            if (expression.getResultSymbol(store_)->getName() != expression.getLvalueSymbol(store_)->getName()) {
                instructions.push_back(std::make_unique<Dereference>(
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getResultSymbol(store_)->getName()));
            }
        } else {
            instructions.push_back(std::make_unique<Dereference>(expression.operandSymbol(store_)->getName(),
                    expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '+':
        break;
    case '-':
        instructions.push_back(std::make_unique<UnaryMinus>(expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        break;
    case '~':
        instructions.push_back(std::make_unique<UnaryNot>(expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        break;
    case '!':
        instructions.push_back(std::make_unique<ZeroCompare>(expression.operandSymbol(store_)->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getTruthyLabel(store_)->getName(), JumpCondition::IF_EQUAL));
        instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol(store_)->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel(store_)->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getTruthyLabel(store_)->getName()));
        instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol(store_)->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel(store_)->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    // Only true array objects need AddressOf. Multi-dim rows already hold a decayed pointer
    // in the result symbol while expression type may still be array.
    if (expression.operandSymbol(store_)->getType().isArray()) {
        instructions.push_back(std::make_unique<AddressOf>(
                expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
    } else {
        instructions.push_back(std::make_unique<Assign>(
                expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const auto* leftSym = expression.leftOperandSymbol(store_);
    const auto* rightSym = expression.rightOperandSymbol(store_);
    const auto* resultSym = expression.getResultSymbol(store_);
    if (!leftSym || !rightSym || !resultSym) {
        return;
    }
    const type::Type leftType = leftSym->getType();
    const type::Type rightType = rightSym->getType();
    const char op = expression.getOperator()->getLexeme().front();

    // Same classification as SA (TypeQuery); pointer math is not integer Add/Sub.
    const type::PointerArithmeticInfo ptrArith = type::classifyPointerArithmetic(leftType, rightType, op);
    switch (ptrArith.form) {
    case type::PointerArithmeticForm::None:
        break;
    case type::PointerArithmeticForm::PtrPlusInt:
    case type::PointerArithmeticForm::IntPlusPtr:
    case type::PointerArithmeticForm::PtrMinusInt: {
        // PointerOffset is always base=pointer, index=integer; swap for int+ptr.
        const bool intLeft = ptrArith.form == type::PointerArithmeticForm::IntPlusPtr;
        const bool subtract = ptrArith.form == type::PointerArithmeticForm::PtrMinusInt;
        const auto* base = intLeft ? rightSym : leftSym;
        const auto* index = intLeft ? leftSym : rightSym;
        instructions.push_back(std::make_unique<PointerOffset>(
                base->getName(), index->getName(), ptrArith.strideBytes, resultSym->getName(), subtract));
        return;
    }
    case type::PointerArithmeticForm::PtrMinusPtr:
        instructions.push_back(std::make_unique<PointerDiff>(
                leftSym->getName(), rightSym->getName(), ptrArith.strideBytes, resultSym->getName()));
        return;
    case type::PointerArithmeticForm::Invalid:
        // SA must diagnose; never silent no-IR in release (NDEBUG).
        throw std::logic_error("pointer arithmetic Invalid should not reach codegen");
    }

    switch (op) {
    case '+':
        instructions.push_back(std::make_unique<Add>(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '-':
        instructions.push_back(std::make_unique<Sub>(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '*':
        instructions.push_back(std::make_unique<Mul>(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '/':
        instructions.push_back(std::make_unique<Div>(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '%':
        instructions.push_back(std::make_unique<Mod>(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    default:
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '<':   // <<
        instructions.push_back(std::make_unique<Shl>(
                    expression.leftOperandSymbol(store_)->getName(),
                    expression.rightOperandSymbol(store_)->getName(),
                    expression.getResultSymbol(store_)->getName()));
        break;
    case '>':   // >>
        instructions.push_back(std::make_unique<Shr>(
                    expression.leftOperandSymbol(store_)->getName(),
                    expression.rightOperandSymbol(store_)->getName(),
                    expression.getResultSymbol(store_)->getName()));
        break;
    default:
        throw std::runtime_error { "unidentified shift operator!" };
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    instructions.push_back(std::make_unique<ValueCompare>(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName()));

    auto truthyLabel = expression.getTruthyLabel(store_)->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE));
    } else if (expression.getOperator()->getLexeme() == "<") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "==") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel(store_)->getName()));
    instructions.push_back(std::make_unique<Label>(truthyLabel));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        instructions.push_back(std::make_unique<And>(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                     expression.getResultSymbol(store_)->getName()));
        break;
    case '|':
        instructions.push_back(std::make_unique<Or>(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                    expression.getResultSymbol(store_)->getName()));
        break;
    case '^':
        instructions.push_back(std::make_unique<Xor>(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                     expression.getResultSymbol(store_)->getName()));
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol(store_)->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol(store_)->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    instructions.push_back(std::make_unique<ZeroCompare>(expression.conditionSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitTrueExpression(*this);
    instructions.push_back(std::make_unique<Assign>(
            expression.trueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel(store_)->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel(store_)->getName()));
    expression.visitFalseExpression(*this);
    instructions.push_back(std::make_unique<Assign>(
            expression.falseSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    auto resultName = expression.getResultSymbol(store_)->getName();
    if (assignmentOperator->getLexeme() == "+=")
        instructions.push_back(std::make_unique<Add>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "-=")
        instructions.push_back(std::make_unique<Sub>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "*=")
        instructions.push_back(std::make_unique<Mul>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "/=")
        instructions.push_back(std::make_unique<Div>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "%=")
        instructions.push_back(std::make_unique<Mod>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "&=")
        instructions.push_back(std::make_unique<And>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "^=")
        instructions.push_back(std::make_unique<Xor>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "|=")
        instructions.push_back(std::make_unique<Or>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "<<=") {
        instructions.push_back(std::make_unique<Shl>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    } else if (assignmentOperator->getLexeme() == ">>=") {
        instructions.push_back(std::make_unique<Shr>(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol(store_)) {
            // Convert into the LHS value temp (correct store width) then write through the address.
            instructions.push_back(std::make_unique<Assign>(
                        expression.rightOperandSymbol(store_)->getName(),
                        resultName
            ));
            instructions.push_back(std::make_unique<LvalueAssign>(
                        resultName,
                        expression.leftOperandLvalueSymbol(store_)->getName()
            ));
        } else {
            instructions.push_back(std::make_unique<Assign>(
                        expression.rightOperandSymbol(store_)->getName(),
                        resultName
            ));
        }
        return;
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator->getLexeme() };
    }

    // Compound assign updated the value temp; write back through pointer lvalues (e.g. *p += 1).
    if (auto* lvalue = expression.leftOperandLvalueSymbol(store_)) {
        instructions.push_back(std::make_unique<LvalueAssign>(resultName, lvalue->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
}

void CodeGeneratingVisitor::visit(ast::Operator&) {
}

void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    if (!statement.getJumpTo(store_)) {
        throw std::runtime_error { "JumpStatement has no target label" };
    }
    instructions.push_back(std::make_unique<Jump>(statement.getJumpTo(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    auto switchResult = statement.expression->getResultSymbol(store_)->getName();
    auto caseTemp = statement.getCaseTemp()->getName();

    for (auto* caseLabel : statement.getCases()) {
        instructions.push_back(std::make_unique<AssignConstant>(
                std::to_string(caseLabel->getCaseValue()), caseTemp));
        instructions.push_back(std::make_unique<ValueCompare>(switchResult, caseTemp));
        instructions.push_back(std::make_unique<Jump>(caseLabel->getLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        instructions.push_back(std::make_unique<Jump>(statement.getDefaultLabel()->getLabel(store_)->getName()));
    } else {
        instructions.push_back(std::make_unique<Jump>(statement.getExitLabel(store_)->getName()));
    }

    statement.body->accept(*this);
    instructions.push_back(std::make_unique<Label>(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    instructions.push_back(std::make_unique<Label>(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    instructions.push_back(std::make_unique<Label>(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    if (!statement.getTarget(store_)) {
        throw std::runtime_error { "GotoStatement has no target label" };
    }
    instructions.push_back(std::make_unique<Jump>(statement.getTarget(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    if (!statement.getLabel(store_)) {
        throw std::runtime_error { "LabeledStatement has no label" };
    }
    instructions.push_back(std::make_unique<Label>(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    instructions.push_back(std::make_unique<Return>(statement.returnExpression->getResultSymbol(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement &statement) { instructions.push_back(std::make_unique<VoidReturn>()); }

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    instructions.push_back(std::make_unique<Jump>(statement.getExitLabel(store_)->getName()));
    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel(store_)->getName()));

    statement.falsyBody->accept(*this);
    instructions.push_back(std::make_unique<Label>(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    if (loop.header->bodyBeforeTest()) {
        // do { body } while (cond); — header visit emits the trailing test + branch.
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopEntry(store_)->getName()));
        loop.body->accept(*this);
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopContinue(store_)->getName()));
        loop.header->accept(*this);
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopExit(store_)->getName()));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);
    // continue target: for-loops place a label before the increment; while reuses entry.
    if (loop.header->getLoopContinue(store_)
            && loop.header->getLoopContinue(store_)->getName() != loop.header->getLoopEntry(store_)->getName()) {
        instructions.push_back(std::make_unique<Label>(loop.header->getLoopContinue(store_)->getName()));
    }
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    instructions.push_back(std::make_unique<Jump>(loop.header->getLoopEntry(store_)->getName()));
    instructions.push_back(std::make_unique<Label>(loop.header->getLoopExit(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.initialization) {
        loopHeader.initialization->accept(*this);
    }

    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry(store_)->getName()));
    if (loopHeader.clause) {
        loopHeader.clause->accept(*this);
        instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol(store_)->getName()));
        instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry(store_)->getName()));
    loopHeader.clause->accept(*this);
    instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Invoked after the body and continue label (see visit(LoopStatement)).
    loopHeader.clause->accept(*this);
    instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol(store_)->getName()));
    instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopEntry(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator& declaration) {
    // Sized arrays are typed in semantic analysis; no IR is emitted for the declarator itself.
    if (declaration.subscriptExpression) {
        declaration.subscriptExpression->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}

void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    // Semantic analysis skips setSymbol when the definition is invalid (e.g. name conflicts).
    if (!function.hasSymbol()) {
        return;
    }

    function.visitDeclarator(*this);

    std::vector<Value> values;
    for (auto& valueSymbol : function.getLocalVariables()) {
        values.push_back( {
                valueSymbol.second.getName(),
                valueSymbol.second.getIndex(),
                // FIXME:
                Type::INTEGRAL,
                valueSymbol.second.getType().getSize()
        });
    }
    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        arguments.push_back( {
                argumentSymbol.getName(),
                argumentSymbol.getIndex(),
                // FIXME:
                Type::INTEGRAL,
                argumentSymbol.getType().getSize()
        });
    }
    instructions.push_back(std::make_unique<StartProcedure>(function.getSymbol()->getName(), std::move(values), std::move(arguments)));

    auto instructionsBak = std::move(instructions);
    function.visitBody(*this);
    auto functionBody = toBasicBlocks(std::move(instructions));
    for (auto& bb : functionBody) {
        if (!bb->terminates()) {
            bb->appendInstruction(std::make_unique<VoidReturn>());
        }
        instructionsBak.push_back(std::move(bb));
    }
    instructions = std::move(instructionsBak);

    instructions.push_back(std::make_unique<EndProcedure>(function.getSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

std::vector<std::unique_ptr<Quadruple>> CodeGeneratingVisitor::getQuadruples() {
    return std::move(instructions);
}

std::vector<std::unique_ptr<BasicBlock>> toBasicBlocks(std::vector<std::unique_ptr<Quadruple>> instructions) {
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks {};

    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>();

    std::vector<std::unique_ptr<Quadruple>> bbInstructions;
    for (auto& instruction : instructions) {
        if (bb->terminates() || instruction->isLabel()) {
            basicBlocks.push_back(std::move(bb));
            bb = std::make_unique<BasicBlock>();
            basicBlocks.back()->setSuccessor(bb.get());
        }
        bb->appendInstruction(std::move(instruction));
    }
    basicBlocks.push_back(std::move(bb));

    return basicBlocks;
}

} // namespace codegen

