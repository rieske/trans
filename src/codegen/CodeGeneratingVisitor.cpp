#include "CodeGeneratingVisitor.h"
#include "ast/InitializerListExpression.h"

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/ObjectAbiType.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"

#include "Instruction.h"

namespace {

codegen::Type valueKindFromType(const type::Type& t) {
    return type::isFloating(t) ? codegen::Type::FLOATING : codegen::Type::INTEGRAL;
}

} // namespace

namespace codegen {

CodeGeneratingVisitor::CodeGeneratingVisitor(symbols::AnnotationStore& store) : store_ { store } {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}

void CodeGeneratingVisitor::emit(Instruction instruction) {
    if (!currentBody_) {
        throw std::logic_error { "CodeGeneratingVisitor: emit outside of a procedure body" };
    }
    currentBody_->push_back(std::move(instruction));
}

std::string CodeGeneratingVisitor::convertedResultName(ast::Expression& expression) {
    auto* result = expression.getResultSymbol(store_);
    // Call-arg array decay: lvalue is the array object, result is the pointer temp.
    if (auto* object = expression.getLvalueSymbol(store_)) {
        if (object->getType().isArray() && result->getType().isPointer()) {
            emit(ir::addressOf(object->getName(), result->getName()));
            return result->getName();
        }
    }
    if (auto* convert = store_.conversion(&expression)) {
        emit(ir::assign(result->getName(), convert->getName()));
        return convert->getName();
    }
    return result->getName();
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
    auto* holder = declarator.getHolder(store_);
    // File-scope variables are initialized in .data; skip children (would emit assigns with no procedure).
    if (declarator.hasInitializer() && holder && holder->isGlobal()) {
        return;
    }
    declarator.visitChildren(*this);
    if (!declarator.hasInitializer()) {
        return;
    }
    assert(holder && "InitializedDeclarator holder required after successful SA");
    const auto& fieldStores = store_.structFieldInits(&declarator);
    if (!fieldStores.empty()) {
        for (const auto& field : fieldStores) {
            emit(ir::fieldAddress(
                    holder->getName(), field.offsetBytes, field.addressName,
                    symbols::AddressBaseMode::LeaObject));
            if (field.zeroInitialize) {
                emit(ir::assignConstant("0", field.sourceName));
            }
            emit(ir::lvalueAssign(field.sourceName, field.addressName));
        }
        return;
    }
    if (declarator.getInitializer()->hasResultSymbol(store_)) {
        emit(ir::assign(
                declarator.getInitializerHolder(store_)->getName(), holder->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);
    if (!arrayAccess.getLvalueSymbol(store_) || !arrayAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* indexPlan = store_.addressPlan(&arrayAccess);
    const auto* index = indexPlan ? symbols::get_if<symbols::IndexPlan>(indexPlan) : nullptr;
    // SA always publishes IndexPlan for successful array access analysis.
    assert(index && "IndexPlan required for array codegen");
    emit(ir::indexAddress(
            arrayAccess.leftOperandSymbol(store_)->getName(),
            arrayAccess.rightOperandSymbol(store_)->getName(),
            index->elementSize,
            arrayAccess.getLvalueSymbol(store_)->getName(),
            index->baseMode));
    if (!arrayAccess.holdsAggregateAddress()) {
        // Load scalar element for rvalue uses; stores use LvalueAssign on the address temp.
        emit(ir::dereference(
                arrayAccess.getLvalueSymbol(store_)->getName(),
                arrayAccess.getLvalueSymbol(store_)->getName(),
                arrayAccess.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
}

void CodeGeneratingVisitor::visit(ast::MemberAccess& memberAccess) {
    memberAccess.getBase()->accept(*this);
    if (!memberAccess.getLvalueSymbol(store_) || !memberAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* plan = store_.addressPlan(&memberAccess);
    const auto* field = plan ? symbols::get_if<symbols::FieldPlan>(plan) : nullptr;
    assert(field && "FieldPlan required for member access codegen");
    const std::string addrTemp = memberAccess.getLvalueSymbol(store_)->getName();
    emit(ir::fieldAddress(
            memberAccess.getBase()->getResultSymbol(store_)->getName(),
            field->fieldOffsetBytes,
            addrTemp,
            field->baseMode));
    if (!memberAccess.holdsAggregateAddress()) {
        emit(ir::dereference(
                addrTemp, addrTemp, memberAccess.getResultSymbol(store_)->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    const symbols::CallPlan* plan = store_.callPlan(&functionCall);
    if (!plan) {
        // SA error path - no IR.
        functionCall.visitOperand(*this);
        functionCall.visitArguments(*this);
        return;
    }

    std::visit(
            [&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::VaStartPlan>
                        || std::is_same_v<T, symbols::VaEndPlan>
                        || std::is_same_v<T, symbols::VaCopyPlan>
                        || std::is_same_v<T, symbols::VaArgPlan>) {
                    functionCall.visitArguments(*this);
                    const auto& args = functionCall.getArgumentList();
                    if constexpr (std::is_same_v<T, symbols::VaStartPlan>) {
                        std::string lastStorage;
                        if (args.size() >= 2) {
                            lastStorage = args[1]->getResultSymbol(store_)->getName();
                        }
                        emit(ir::vaStart(args[0]->getResultSymbol(store_)->getName(),
                                std::move(lastStorage)));
                    } else if constexpr (std::is_same_v<T, symbols::VaEndPlan>) {
                        emit(ir::vaEnd());
                    } else if constexpr (std::is_same_v<T, symbols::VaCopyPlan>) {
                        emit(ir::vaCopy(args[0]->getResultSymbol(store_)->getName(),
                                args[1]->getResultSymbol(store_)->getName()));
                    } else {
                        type::Type retTy = functionCall.getResultSymbol(store_)->getType();
                        int accessSize = retTy.isPointer() ? 8 : retTy.getSize();
                        if (accessSize < 1 || accessSize > 8) {
                            accessSize = 8;
                        }
                        emit(ir::vaArg(args[0]->getResultSymbol(store_)->getName(),
                                functionCall.getResultSymbol(store_)->getName(), accessSize,
                                type::isFloating(retTy), type::valueIsSigned(retTy)));
                    }
                } else {
                    functionCall.visitOperand(*this);
                    functionCall.visitArguments(*this);
                    for (auto& expression : functionCall.getArgumentList()) {
                        emit(ir::argument(convertedResultName(*expression)));
                    }
                    std::string memoryReturnDest;
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        if (type::object_abi::productEmitsMemoryReturn(
                                    functionCall.getType(), symbols::callIsVariadic(*plan))) {
                            memoryReturnDest = functionCall.getResultSymbol(store_)->getName();
                        }
                    }
                    emit(ir::call(symbols::callCalleeName(*plan), symbols::isIndirectCall(*plan),
                            memoryReturnDest));
                    if (functionCall.hasResultSymbol(store_) && !functionCall.getType().isVoid()) {
                        emit(ir::retrieve(functionCall.getResultSymbol(store_)->getName(),
                                !memoryReturnDest.empty()));
                    }
                }
            },
            *plan);
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (identifier.hasFoldedConstant()) {
        assert(identifier.hasResultSymbol(store_) && "folded enumerator needs Result temp");
        emit(ir::assignConstant(
                std::to_string(identifier.getFoldedConstant()),
                identifier.getResultSymbol(store_)->getName()));
        return;
    }
    // Function designators: plan holds the label; Result is the address temp.
    if (const auto* plan = store_.addressPlan(&identifier)) {
        if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
            assert(identifier.hasResultSymbol(store_) && "designator Result required for FunctionAddress");
            emit(ir::functionAddress(
                    d->functionName, identifier.getResultSymbol(store_)->getName()));
            return;
        }
    }
    assert(!identifier.holdsFunctionDesignator()
            && "designator form without FunctionDesignatorPlan on the store");
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so suffixes never reach the assembler raw.
    std::string immediate;
    if (type::isFloating(constant.expressionType())) {
        if (!util::floatingLiteralImmediate(constant.getValue(), immediate)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
    } else {
        long value;
        if (constant.evaluateConstant(value)) {
            immediate = util::wordImmediate(static_cast<unsigned long long>(value));
        } else {
            immediate = constant.getValue();
        }
    }
    emit(ir::assignConstant(immediate, constant.getResultSymbol(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    emit(ir::assignLabelAddress(
            stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol(store_)->getName()));
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

    auto* pre = expression.getPreOperationSymbol(store_);
    assert(pre && "Postfix PreOperation required after successful SA");
    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    auto preOperationSymbol = pre->getName();
    emit(ir::assign(resultSymbolName, preOperationSymbol));

    const int step = incDecStepBytes(expression.getResultSymbol(store_)->getType());
    if (expression.getOperator()->getLexeme() == "++") {
        emit(ir::inc(resultSymbolName, step));
    } else if (expression.getOperator()->getLexeme() == "--") {
        emit(ir::dec(resultSymbolName, step));
    }

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
        emit(ir::lvalueAssign(resultSymbolName, lvalue->getName()));
    }

    expression.setResultSymbol(store_, *pre);
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    const int step = incDecStepBytes(expression.getResultSymbol(store_)->getType());
    if (expression.getOperator()->getLexeme() == "++") {
        emit(ir::inc(resultSymbolName, step));
    } else if (expression.getOperator()->getLexeme() == "--") {
        emit(ir::dec(resultSymbolName, step));
    }

    if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
        emit(ir::lvalueAssign(resultSymbolName, lvalue->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    if (expression.getOperator()->getLexeme() == "sizeof") {
        // Operand is unevaluated at runtime; emit the folded size constant.
        emit(ir::assignConstant(
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
            emit(ir::assign(
                    lvalue->getName(), expression.getResultSymbol(store_)->getName()));
        } else {
            emit(ir::addressOf(
                    expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '*':
        if (expression.operandSymbol(store_)->getType().isPointer()) {
            // *fp for pointer-to-function: SA keeps the pointer value (no memory load).
            if (type::isPointerToBareFunction(expression.operandSymbol(store_)->getType())) {
                if (expression.operandSymbol(store_)->getName() != expression.getResultSymbol(store_)->getName()) {
                    emit(ir::assign(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
                break;
            }
            // Already an address (pointer or multi-dim decayed row).
            if (expression.getResultSymbol(store_)->getName() == expression.getLvalueSymbol(store_)->getName()) {
                // Address-only multi-dim *a: just materialize &array into the temp if needed.
                // Result and lvalue share the address temp; operand is the array object.
                if (expression.operandType().isArray()) {
                    emit(ir::addressOf(
                            expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
                } else {
                    emit(ir::assign(
                            expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
                }
            } else {
                emit(ir::dereference(expression.operandSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
            }
        } else if (expression.operandType().isArray()) {
            // True array object: &a then optional load.
            emit(ir::addressOf(
                    expression.operandSymbol(store_)->getName(), expression.getLvalueSymbol(store_)->getName()));
            if (expression.getResultSymbol(store_)->getName() != expression.getLvalueSymbol(store_)->getName()) {
                emit(ir::dereference(
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getLvalueSymbol(store_)->getName(),
                        expression.getResultSymbol(store_)->getName()));
            }
        } else {
            emit(ir::dereference(expression.operandSymbol(store_)->getName(),
                    expression.getLvalueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    case '+':
        break;
    case '-':
        emit(ir::unaryMinus(expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        break;
    case '~':
        emit(ir::unaryNot(expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
        break;
    case '!':
        emit(ir::zeroCompare(expression.operandSymbol(store_)->getName()));
        emit(ir::jump(expression.getTruthyLabel(store_)->getName(), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
        emit(ir::jump(expression.getFalsyLabel(store_)->getName()));
        emit(ir::label(expression.getTruthyLabel(store_)->getName()));
        emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
        emit(ir::label(expression.getFalsyLabel(store_)->getName()));
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
        emit(ir::addressOf(
                expression.operandSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
    } else {
        emit(ir::assign(
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
        emit(ir::pointerOffset(
                base->getName(), index->getName(), ptrArith.strideBytes, resultSym->getName(), subtract));
        return;
    }
    case type::PointerArithmeticForm::PtrMinusPtr:
        emit(ir::pointerDiff(
                leftSym->getName(), rightSym->getName(), ptrArith.strideBytes, resultSym->getName()));
        return;
    case type::PointerArithmeticForm::Invalid:
        // SA must diagnose; never silent no-IR in release (NDEBUG).
        throw std::logic_error("pointer arithmetic Invalid should not reach codegen");
    }

    switch (op) {
    case '+':
        emit(ir::add(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '-':
        emit(ir::sub(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '*':
        emit(ir::mul(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '/':
        emit(ir::div(leftSym->getName(), rightSym->getName(), resultSym->getName()));
        break;
    case '%':
        emit(ir::mod(leftSym->getName(), rightSym->getName(), resultSym->getName()));
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
        emit(ir::shl(
                    expression.leftOperandSymbol(store_)->getName(),
                    expression.rightOperandSymbol(store_)->getName(),
                    expression.getResultSymbol(store_)->getName()));
        break;
    case '>':   // >>
        emit(ir::shr(
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

    emit(ir::valueCompare(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName()));

    auto truthyLabel = expression.getTruthyLabel(store_)->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE));
    } else if (expression.getOperator()->getLexeme() == "<") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "==") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        emit(ir::jump(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::jump(expression.getFalsyLabel(store_)->getName()));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::label(expression.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        emit(ir::andOp(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                     expression.getResultSymbol(store_)->getName()));
        break;
    case '|':
        emit(ir::orOp(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                    expression.getResultSymbol(store_)->getName()));
        break;
    case '^':
        emit(ir::xorOp(expression.leftOperandSymbol(store_)->getName(), expression.rightOperandSymbol(store_)->getName(),
                                                     expression.getResultSymbol(store_)->getName()));
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.leftOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(expression.rightOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.leftOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(expression.rightOperandSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    expression.visitCondition(*this);
    emit(ir::zeroCompare(expression.conditionSymbol(store_)->getName()));
    emit(ir::jump(expression.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    expression.visitTrueExpression(*this);
    emit(ir::assign(
            expression.trueSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));
    emit(ir::jump(expression.getExitLabel(store_)->getName()));

    emit(ir::label(expression.getFalsyLabel(store_)->getName()));
    expression.visitFalseExpression(*this);
    emit(ir::assign(
            expression.falseSymbol(store_)->getName(), expression.getResultSymbol(store_)->getName()));

    emit(ir::label(expression.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    auto resultName = expression.getResultSymbol(store_)->getName();
    if (assignmentOperator->getLexeme() == "+=")
        emit(ir::add(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "-=")
        emit(ir::sub(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "*=")
        emit(ir::mul(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "/=")
        emit(ir::div(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "%=")
        emit(ir::mod(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "&=")
        emit(ir::andOp(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "^=")
        emit(ir::xorOp(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "|=")
        emit(ir::orOp(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    else if (assignmentOperator->getLexeme() == "<<=") {
        emit(ir::shl(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    } else if (assignmentOperator->getLexeme() == ">>=") {
        emit(ir::shr(
                    resultName,
                    expression.rightOperandSymbol(store_)->getName(),
                    resultName
        ));
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol(store_)) {
            // Convert into the LHS value temp (correct store width) then write through the address.
            emit(ir::assign(
                        expression.rightOperandSymbol(store_)->getName(),
                        resultName
            ));
            emit(ir::lvalueAssign(
                        resultName,
                        expression.leftOperandLvalueSymbol(store_)->getName()
            ));
        } else {
            emit(ir::assign(
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
        emit(ir::lvalueAssign(resultName, lvalue->getName()));
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
    emit(ir::jump(statement.getJumpTo(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::SwitchStatement& statement) {
    statement.expression->accept(*this);

    auto switchResult = statement.expression->getResultSymbol(store_)->getName();
    auto caseTemp = statement.getCaseTemp(store_)->getName();

    for (auto* caseLabel : statement.getCases()) {
        emit(ir::assignConstant(
                std::to_string(caseLabel->getCaseValue()), caseTemp));
        emit(ir::valueCompare(switchResult, caseTemp));
        emit(ir::jump(caseLabel->getLabel(store_)->getName(), JumpCondition::IF_EQUAL));
    }

    if (statement.getDefaultLabel()) {
        emit(ir::jump(statement.getDefaultLabel()->getLabel(store_)->getName()));
    } else {
        emit(ir::jump(statement.getExitLabel(store_)->getName()));
    }

    statement.body->accept(*this);
    emit(ir::label(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::CaseLabel& statement) {
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::DefaultLabel& statement) {
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::GotoStatement& statement) {
    if (!statement.getTarget(store_)) {
        throw std::runtime_error { "GotoStatement has no target label" };
    }
    emit(ir::jump(statement.getTarget(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LabeledStatement& statement) {
    if (!statement.getLabel(store_)) {
        throw std::runtime_error { "LabeledStatement has no label" };
    }
    emit(ir::label(statement.getLabel(store_)->getName()));
    statement.statement->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    emit(ir::ret(convertedResultName(*statement.returnExpression)));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement &statement) { emit(ir::voidReturn()); }

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    emit(ir::label(statement.getFalsyLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    emit(ir::zeroCompare(statement.testExpression->getResultSymbol(store_)->getName()));
    emit(ir::jump(statement.getFalsyLabel(store_)->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    emit(ir::jump(statement.getExitLabel(store_)->getName()));
    emit(ir::label(statement.getFalsyLabel(store_)->getName()));

    statement.falsyBody->accept(*this);
    emit(ir::label(statement.getExitLabel(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    if (loop.header->bodyBeforeTest()) {
        // do { body } while (cond); — header visit emits the trailing test + branch.
        emit(ir::label(loop.header->getLoopEntry(store_)->getName()));
        loop.body->accept(*this);
        emit(ir::label(loop.header->getLoopContinue(store_)->getName()));
        loop.header->accept(*this);
        emit(ir::label(loop.header->getLoopExit(store_)->getName()));
        return;
    }

    loop.header->accept(*this);
    loop.body->accept(*this);
    // continue target: for-loops place a label before the increment; while reuses entry.
    if (loop.header->getLoopContinue(store_)
            && loop.header->getLoopContinue(store_)->getName() != loop.header->getLoopEntry(store_)->getName()) {
        emit(ir::label(loop.header->getLoopContinue(store_)->getName()));
    }
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    emit(ir::jump(loop.header->getLoopEntry(store_)->getName()));
    emit(ir::label(loop.header->getLoopExit(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    if (loopHeader.initialization) {
        loopHeader.initialization->accept(*this);
    }

    emit(ir::label(loopHeader.getLoopEntry(store_)->getName()));
    if (loopHeader.clause) {
        loopHeader.clause->accept(*this);
        emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
        emit(ir::jump(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
    }
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    emit(ir::label(loopHeader.getLoopEntry(store_)->getName()));
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
    emit(ir::jump(loopHeader.getLoopExit(store_)->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::DoWhileLoopHeader& loopHeader) {
    // Invoked after the body and continue label (see visit(LoopStatement)).
    loopHeader.clause->accept(*this);
    emit(ir::zeroCompare(loopHeader.clause->getResultSymbol(store_)->getName()));
    emit(ir::jump(loopHeader.getLoopEntry(store_)->getName(), JumpCondition::IF_NOT_EQUAL));
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
                valueKindFromType(valueSymbol.second.getType()),
                valueSymbol.second.getType().getSize()
        });
    }
    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        arguments.push_back( {
                argumentSymbol.getName(),
                argumentSymbol.getIndex(),
                valueKindFromType(argumentSymbol.getType()),
                argumentSymbol.getType().getSize()
        });
    }
    Procedure procedure;
    procedure.name = function.getSymbol()->getName();
    procedure.frame.locals = std::move(values);
    procedure.frame.arguments = std::move(arguments);
    const bool variadic = function.getSymbol()->getType().isVariadic();
    procedure.memoryReturn = type::object_abi::productEmitsMemoryReturn(
            function.getSymbol()->returnType(), variadic);
    procedure.variadic = variadic;

    std::vector<Instruction>* previousBody = currentBody_;
    currentBody_ = &procedure.body;
    function.visitBody(*this);
    currentBody_ = previousBody;

    module_.procedures.push_back(std::move(procedure));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

IntermediateRepresentation CodeGeneratingVisitor::takeIr() {
    return std::move(module_);
}

} // namespace codegen

