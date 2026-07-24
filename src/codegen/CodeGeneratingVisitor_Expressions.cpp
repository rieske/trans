#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "ast/ArrayAccess.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "types/ObjectAbi.h"
#include "FrameLayout.h"

#include "quadruples/Assign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Retrieve.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Inc.h"
#include "quadruples/Dec.h"
#include "quadruples/AddressOf.h"
#include "quadruples/FunctionAddress.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/BitwiseNot.h"
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
#include "quadruples/FieldAccess.h"
#include "quadruples/IndexAddress.h"
#include "quadruples/Truncate.h"
#include "quadruples/BuiltinOp.h"
#include "quadruples/VaOp.h"
#include "ast/MemberAccess.h"
#include "ast/IdentifierExpression.h"
#include "ast/FunctionCall.h"
#include "ast/Expression.h"

namespace codegen {




void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto resultSymbolName = expression.getResultSymbol()->getName();
    auto preOperationSymbol = expression.getPreOperationSymbol()->getName();
    instructions.push_back(std::make_unique<Assign>(resultSymbolName, preOperationSymbol));

    const int amount = pointerIncrementAmount(expression.operandValueType());
    if (expression.getOperator()->getKind() == ast::OperatorKind::Inc) {
        instructions.push_back(std::make_unique<Inc>(resultSymbolName, amount));
    } else if (expression.getOperator()->getKind() == ast::OperatorKind::Dec) {
        instructions.push_back(std::make_unique<Dec>(resultSymbolName, amount));
    }

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (auto* lvalue = expression.operandLvalueSymbol()) {
        instructions.push_back(std::make_unique<LvalueAssign>(
                resultSymbolName, lvalue->getName(),
                type::memoryAccessSizeBytes(expression.operandValueType())));
    }

    expression.setTypedResult(*expression.getPreOperationSymbol());
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto resultSymbolName = expression.getResultSymbol()->getName();
    const int amount = pointerIncrementAmount(expression.operandValueType());
    if (expression.getOperator()->getKind() == ast::OperatorKind::Inc) {
        instructions.push_back(std::make_unique<Inc>(resultSymbolName, amount));
    } else if (expression.getOperator()->getKind() == ast::OperatorKind::Dec) {
        instructions.push_back(std::make_unique<Dec>(resultSymbolName, amount));
    }

    if (auto* lvalue = expression.operandLvalueSymbol()) {
        instructions.push_back(std::make_unique<LvalueAssign>(
                resultSymbolName, lvalue->getName(),
                type::memoryAccessSizeBytes(expression.operandValueType())));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (op == OperatorKind::Sizeof
            || (op == OperatorKind::AddressOf && expression.getSizeofValue() >= 0)) {
        instructions.push_back(std::make_unique<AssignConstant>(
                std::to_string(expression.getSizeofValue()), expression.getResultSymbol()->getName()));
        return;
    }

    if (op == OperatorKind::AddressOf) {
        emitAddressOf(*expression.getOperandExpression(), expression.getResultSymbol()->getName());
        return;
    }

    generateExpression(*expression.getOperandExpression());

    switch (op) {
    case OperatorKind::Deref: {
        // *fp on a function pointer is a no-op (result already aliases the pointer).
        if (expression.getResultSymbol() && expression.operandSymbol()
                && expression.getResultSymbol()->getName() == expression.operandSymbol()->getName()) {
            break;
        }
        if (expression.operandType().isArray()) {
            // *arr is arr[0]. Member arrays may already hold &arr[0] as the result.
            const std::string ptrName = expression.getLvalueSymbol()->getName();
            const type::Type resType = expression.valueType();
            bool alreadyAddress = false;
            if (auto* member = dynamic_cast<ast::MemberAccess*>(
                        expression.getOperandExpression())) {
                if (member->expressionType().isArray()) {
                    alreadyAddress = true;
                }
            }
            if (alreadyAddress) {
                instructions.push_back(std::make_unique<Dereference>(
                        expression.operandSymbol()->getName(),
                        ptrName,
                        expression.getResultSymbol()->getName(),
                        type::memoryAccessSizeBytes(resType), type::memoryAccessIsSigned(resType)));
            } else {
                instructions.push_back(std::make_unique<AddressOf>(
                        expression.operandSymbol()->getName(), ptrName));
                instructions.push_back(std::make_unique<Dereference>(
                        ptrName, ptrName, expression.getResultSymbol()->getName(),
                        type::memoryAccessSizeBytes(resType), type::memoryAccessIsSigned(resType)));
            }
        } else {
            const type::Type& resType = expression.getResultSymbol()->getType();
            instructions.push_back(std::make_unique<Dereference>(
                    expression.operandSymbol()->getName(),
                    expression.getLvalueSymbol()->getName(),
                    expression.getResultSymbol()->getName(),
                    type::memoryAccessSizeBytes(resType), type::memoryAccessIsSigned(resType)));
        }
        break;
    }
    case OperatorKind::Add:
        break;
    case OperatorKind::Sub:
        instructions.push_back(std::make_unique<UnaryMinus>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
        narrowIntegralResult(expression.getResultSymbol()->getType(), expression.getResultSymbol()->getName());
        break;
    case OperatorKind::BitNot:
        instructions.push_back(std::make_unique<BitwiseNot>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
        narrowIntegralResult(expression.getResultSymbol()->getType(), expression.getResultSymbol()->getName());
        break;
    case OperatorKind::LogicalNot:
        instructions.push_back(std::make_unique<ZeroCompare>(expression.operandSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getTruthyLabel()->getName(), JumpCondition::IF_EQUAL));
        instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getTruthyLabel()->getName()));
        instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    generateExpression(*expression.getOperandExpression());
    const type::Type& target = expression.getResultSymbol()->getType();
    const type::Type& source = expression.operandSymbol()->getType();
    // Array-to-pointer cast: (T*)arr must yield &arr[0], not the first word of
    // the array contents (git sha1dc: (const char*)(sha1_padding) where pad[0]==0x80).
    if (source.isArray() && target.isPointer()) {
        if (auto* lvalue = expression.getOperandExpression()->getLvalueSymbol()) {
            // Member / subscript array: address already materialized as lvalue.
            instructions.push_back(std::make_unique<Assign>(
                    lvalue->getName(), expression.getResultSymbol()->getName()));
        } else {
            instructions.push_back(std::make_unique<AddressOf>(
                    expression.operandSymbol()->getName(),
                    expression.getResultSymbol()->getName()));
        }
        return;
    }
    // Assign converts between FLOATING and INTEGRAL when Value types differ (SSE).
    instructions.push_back(std::make_unique<Assign>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
    // Integer casts must narrow: (unsigned char)(-1) is 255, not all-ones.
    // Locals pad sub-word types to 8 bytes, so emit an explicit truncate.
    if (target.kind() == type::TypeKind::Primitive && !type::isFloating(target)) {
        const int size = target.getSize();
        if (size > 0 && size < 8) {
            instructions.push_back(std::make_unique<Truncate>(
                    expression.getResultSymbol()->getName(),
                    size,
                    type::valueIsSigned(target)));
        }
    }
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    // C usual arithmetic conversions: if either side is unsigned, / and % are unsigned.
    // Signed idiv traps when quotient has high bit set (git st_mult: SIZE_MAX / a).
    // Value types after decay (not expressionType: member arrays stay T[N] there).
    const bool unsignedDiv =
            type::isUnsignedSide(expression.leftValueType())
            || type::isUnsignedSide(expression.rightValueType());
    const OperatorKind op = expression.getOperator()->getKind();
    const std::string& left = expression.leftOperandSymbol()->getName();
    const std::string& right = expression.rightOperandSymbol()->getName();
    const std::string& result = expression.getResultSymbol()->getName();

    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        if (expression.isPointerDifference()) {
            // (p - q) / sizeof(*p) -> element count (C 6.5.6).
            const int scale = expression.getPointerScale();
            const std::string* scaleTemp = expression.getScaleTempName();
            emitBinaryOp(instructions, OperatorKind::Sub, left, right, result);
            if (scaleTemp && scale > 1) {
                instructions.push_back(std::make_unique<AssignConstant>(std::to_string(scale), *scaleTemp));
                emitBinaryOp(instructions, OperatorKind::Div, result, *scaleTemp, result, false);
            }
        } else if (const std::string* scaleTemp = expression.getScaleTempName()) {
            // Pointer +/- integer: scaled = index * sizeof(pointee); then ptr +/- scaled.
            const int scale = expression.getPointerScale();
            const std::string& indexName = expression.pointerIsLeftOperand() ? right : left;
            const std::string& ptrName = expression.pointerIsLeftOperand() ? left : right;
            instructions.push_back(std::make_unique<AssignConstant>(std::to_string(scale), *scaleTemp));
            emitBinaryOp(instructions, OperatorKind::Mul, indexName, *scaleTemp, *scaleTemp);
            emitBinaryOp(instructions, op, ptrName, *scaleTemp, result);
        } else {
            emitBinaryOp(instructions, op, left, right, result);
        }
    } else if (op == OperatorKind::Mul || op == OperatorKind::Div || op == OperatorKind::Mod) {
        emitBinaryOp(instructions, op, left, right, result, unsignedDiv);
    } else {
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
    // Pointer results stay full-width; integral sub-word results need re-extension.
    if (!expression.getResultSymbol()->getType().isPointer()) {
        narrowIntegralResult(expression.getResultSymbol()->getType(), result);
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const OperatorKind op = expression.getOperator()->getKind();
    // C 6.5.7: unsigned >> is logical (zero-fill); signed is arithmetic (SAR).
    const type::Type shiftedType = expression.valueType();
    const bool logical = type::isIntegral(shiftedType) && !type::valueIsSigned(shiftedType);
    if (op != OperatorKind::Shl && op != OperatorKind::Shr) {
        throw std::runtime_error { "unidentified shift operator!" };
    }
    emitBinaryOp(instructions, op,
            expression.leftOperandSymbol()->getName(),
            expression.rightOperandSymbol()->getName(),
            expression.getResultSymbol()->getName(),
            false, logical);
    narrowIntegralResult(expression.getResultSymbol()->getType(), expression.getResultSymbol()->getName());
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    // C usual arithmetic conversions: if either side is unsigned (or pointer),
    // relational compare is unsigned. Using signed jg on size_t max (~0) makes
    // `b > max - a` true for small b (git unsigned_add_overflows / st_add).
    auto typeWidth = [](const type::Type& t) {
        if (t.isPointer() || t.isArray()) {
            return 8;
        }
        if (t.isPrimitive()) {
            return t.getSize();
        }
        return 8;
    };
    const bool unsignedCompare =
            type::isUnsignedSide(expression.leftValueType())
            || type::isUnsignedSide(expression.rightValueType());

    // Mixed signed/unsigned equality must convert both to the common unsigned
    // width first. Otherwise unsigned int 0xffffffff (zero-ext in a 64-bit reg)
    // does not equal signed -1 (0xffffffffffffffff) - git: opt.sign == -1.
    if (unsignedCompare) {
        const int leftW = typeWidth(expression.leftValueType());
        const int rightW = typeWidth(expression.rightValueType());
        const int common = leftW > rightW ? leftW : rightW;
        if (common > 0 && common < 8) {
            instructions.push_back(std::make_unique<Truncate>(
                    expression.leftOperandSymbol()->getName(), common, false));
            instructions.push_back(std::make_unique<Truncate>(
                    expression.rightOperandSymbol()->getName(), common, false));
        }
    }

    instructions.push_back(std::make_unique<ValueCompare>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName()));

    auto truthyLabel = expression.getTruthyLabel()->getName();
    using ast::OperatorKind;
    JumpCondition cond;
    switch (expression.getOperator()->getKind()) {
    case OperatorKind::Gt:
        cond = unsignedCompare ? JumpCondition::IF_ABOVE_U : JumpCondition::IF_ABOVE;
        break;
    case OperatorKind::Lt:
        cond = unsignedCompare ? JumpCondition::IF_BELOW_U : JumpCondition::IF_BELOW;
        break;
    case OperatorKind::Le:
        cond = unsignedCompare ? JumpCondition::IF_BELOW_OR_EQUAL_U : JumpCondition::IF_BELOW_OR_EQUAL;
        break;
    case OperatorKind::Ge:
        cond = unsignedCompare ? JumpCondition::IF_ABOVE_OR_EQUAL_U : JumpCondition::IF_ABOVE_OR_EQUAL;
        break;
    case OperatorKind::Eq:
        cond = JumpCondition::IF_EQUAL;
        break;
    case OperatorKind::Ne:
        cond = JumpCondition::IF_NOT_EQUAL;
        break;
    default:
        throw std::runtime_error { "unidentified comparison operator" };
    }
    instructions.push_back(std::make_unique<Jump>(truthyLabel, cond));

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
    instructions.push_back(std::make_unique<Label>(truthyLabel));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const ast::OperatorKind op = expression.getOperator()->getKind();
    if (op != ast::OperatorKind::BitAnd && op != ast::OperatorKind::BitOr && op != ast::OperatorKind::BitXor) {
        throw std::runtime_error { "no semantic actions defined for bitwise operator: "
                + expression.getOperator()->getLexeme() };
    }
    emitBinaryOp(instructions, op,
            expression.leftOperandSymbol()->getName(),
            expression.rightOperandSymbol()->getName(),
            expression.getResultSymbol()->getName());
    narrowIntegralResult(expression.getResultSymbol()->getType(), expression.getResultSymbol()->getName());
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));

    generateExpression(*expression.getRightOperand());

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));

    generateExpression(*expression.getRightOperand());

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    generateExpression(*expression.getCondition());
    instructions.push_back(std::make_unique<ZeroCompare>(expression.conditionSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    const bool producesValue = expression.hasResultSymbol()
            && !expression.getResultSymbol()->getType().isVoid();

    std::string trueName = generateExpression(*expression.getTrueExpression());
    if (producesValue) {
        instructions.push_back(std::make_unique<Assign>(
                trueName, expression.getResultSymbol()->getName()));
    }
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
    std::string falseName = generateExpression(*expression.getFalseExpression());
    if (producesValue) {
        instructions.push_back(std::make_unique<Assign>(
                falseName, expression.getResultSymbol()->getName()));
    }

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const OperatorKind assignKind = expression.getOperator()->getKind();
    auto resultName = expression.getResultSymbol()->getName();

    if (assignKind == OperatorKind::Assign) {
        if (expression.leftOperandLvalueSymbol()) {
            // Convert into the assignment result temp first, then store.
            // LvalueAssign alone bitcasts; float->int must cvttsd2si before the
            // memory write (git: d->rename_score = p->score * 100 / MAX_SCORE).
            // C 6.5.16: result is the value stored - leftOperandSymbol still holds
            // the pre-store load; refresh it so (p = malloc(n), p) sees the new
            // pointer (git DUP_ARRAY in copy_pathspec).
            instructions.push_back(std::make_unique<Assign>(
                        expression.rightOperandSymbol()->getName(),
                        resultName
            ));
            instructions.push_back(std::make_unique<LvalueAssign>(
                        resultName,
                        expression.leftOperandLvalueSymbol()->getName(),
                        type::memoryAccessSizeBytes(expression.leftValueType())
            ));
        } else {
            instructions.push_back(std::make_unique<Assign>(
                        expression.rightOperandSymbol()->getName(),
                        resultName
            ));
        }
        return;
    }

    const OperatorKind baseOp = ast::compoundAssignBase(assignKind);
    if (baseOp == OperatorKind::Unknown) {
        throw std::runtime_error { "unidentified assignment operator: "
                + expression.getOperator()->getLexeme() };
    }

    // Pointer compound assign: scale integer RHS by pointee size (setPointerArithmetic).
    std::string rhsName = expression.rightOperandSymbol()->getName();
    if ((baseOp == OperatorKind::Add || baseOp == OperatorKind::Sub)
            && expression.getScaleTempName()) {
        const std::string* scaleTemp = expression.getScaleTempName();
        const int scale = expression.getPointerScale();
        instructions.push_back(std::make_unique<AssignConstant>(std::to_string(scale), *scaleTemp));
        emitBinaryOp(instructions, OperatorKind::Mul, rhsName, *scaleTemp, *scaleTemp);
        rhsName = *scaleTemp;
    }

    const bool unsignedDiv =
            type::isUnsignedSide(expression.leftValueType())
            || type::isUnsignedSide(expression.rightValueType());
    const type::Type lhsType = expression.leftValueType();
    const bool logicalShr = type::isIntegral(lhsType) && !type::valueIsSigned(lhsType);
    emitBinaryOp(instructions, baseOp, resultName, rhsName, resultName, unsignedDiv, logicalShr);

    // Compound assign is a read-modify-write of a C type width (often < 8).
    // Without truncate, e.g. uint32_t x = 0xffffffff; x += 1 leaves bit 32 set
    // in the register, which poisons later shifts/rotates (git sha1dc e += ...).
    if (!expression.getResultSymbol()->getType().isPointer()) {
        narrowIntegralResult(expression.getResultSymbol()->getType(), resultName);
    }

    // Compound assign updated the value temp; write back through pointer lvalues (e.g. *p += 1).
    if (auto* lvalue = expression.leftOperandLvalueSymbol()) {
        instructions.push_back(std::make_unique<LvalueAssign>(
                resultName, lvalue->getName(), type::memoryAccessSizeBytes(expression.leftValueType())));
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());
}

void CodeGeneratingVisitor::visit(ast::Operator&) {
}

void CodeGeneratingVisitor::visit(ast::MemberAccess& expression) {
    generateExpression(*expression.getBase());
    bool baseIsPointer = expression.isArrow();
    std::string baseName = expression.getBaseResultSymbol()->getName();
    // a[i].m / nested field: base may already be an address in getLvalueSymbol.
    resolveMemberBase(baseName, baseIsPointer, expression.getBase());
    const int offset = expression.getMemberOffset();
    const std::string addrName = expression.getFieldAddressSymbol()->getName();
    const std::string resultName = expression.getResultSymbol()->getName();
    // Materialize field address (dot: &object+off; arrow: pointer+off).
    instructions.push_back(std::make_unique<FieldAddress>(baseName, offset, addrName, baseIsPointer));
    // Array members are not loaded: expressionType stays array, valueType is
    // pointer-to-element. Copy address into the result temp for rvalue uses
    // (git FLEX_ALLOC_MEM: memcpy((void *)(x)->flexname, ...)).
    if (expression.expressionType().isArray()) {
        instructions.push_back(std::make_unique<Assign>(addrName, resultName));
    } else {
        instructions.push_back(std::make_unique<Dereference>(
                addrName, addrName, resultName,
                type::memoryAccessSizeBytes(expression.valueType()),
                type::memoryAccessIsSigned(expression.valueType())));
    }
}

} // namespace codegen
