#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "Instruction.h"
#include "ast/ArrayAccess.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "types/ObjectAbiType.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"
#include "FrameLayout.h"

#include "ast/MemberAccess.h"
#include "ast/IdentifierExpression.h"
#include "ast/Expression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/CompoundLiteralExpression.h"
#include "builtins/BuiltinRegistry.h"
#include "util/ProductApprox.h"

namespace codegen {




void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto resultSymbolName = expression.result(store_)->getName();
    auto preOperationSymbol = store_.value(&expression, symbols::ValueSlot::PreOperation)->getName();
    emit(ir::assign(resultSymbolName, preOperationSymbol));

    const int amount = pointerIncrementAmount(expression.getOperandExpression()->valueType(store_));
    if (expression.getOperator()->getKind() == ast::OperatorKind::Inc) {
        emit(ir::inc(resultSymbolName, amount));
    } else if (expression.getOperator()->getKind() == ast::OperatorKind::Dec) {
        emit(ir::dec(resultSymbolName, amount));
    }

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (auto* lvalue = expression.getOperandExpression()->lvalueAnnotation(store_)) {
        emit(ir::lvalueAssign(
                resultSymbolName, lvalue->getName(),
                type::memoryAccessSizeBytes(expression.getOperandExpression()->valueType(store_))));
    }

    expression.setTypeAndResult(store_, *store_.value(&expression, symbols::ValueSlot::PreOperation));
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto resultSymbolName = expression.result(store_)->getName();
    const int amount = pointerIncrementAmount(expression.getOperandExpression()->valueType(store_));
    if (expression.getOperator()->getKind() == ast::OperatorKind::Inc) {
        emit(ir::inc(resultSymbolName, amount));
    } else if (expression.getOperator()->getKind() == ast::OperatorKind::Dec) {
        emit(ir::dec(resultSymbolName, amount));
    }

    if (auto* lvalue = expression.getOperandExpression()->lvalueAnnotation(store_)) {
        emit(ir::lvalueAssign(
                resultSymbolName, lvalue->getName(),
                type::memoryAccessSizeBytes(expression.getOperandExpression()->valueType(store_))));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (expression.hasFoldedInteger()) {
        // sizeof result or offsetof (&((T*)0)->m) - both are SA-folded integers.
        emit(ir::assignConstant(
                std::to_string(expression.foldedInteger()), expression.result(store_)->getName()));
        return;
    }

    if (op == OperatorKind::AddressOf) {
        emitAddressOf(*expression.getOperandExpression(), expression.result(store_)->getName());
        return;
    }

    generateExpression(*expression.getOperandExpression());

    switch (op) {
    case OperatorKind::Deref: {
        // *fp on a function pointer is a no-op (result already aliases the pointer).
        if (expression.result(store_) && expression.getOperandExpression()->result(store_)
                && expression.result(store_)->getName() == expression.getOperandExpression()->result(store_)->getName()) {
            break;
        }
        // Materialize a pointer to the pointee, then one load path.
        auto* operand = expression.getOperandExpression();
        const std::string ptrName = expression.lvalueAnnotation(store_)->getName();
        const type::Type resType = expression.result(store_)->getType();
        std::string addrSrc = operand->result(store_)->getName();
        if (expression.operandType().isArray()) {
            // *arr is arr[0]. Dual-type decayed arrays already hold &elem[0].
            if (!operand->holdsAggregateAddress()) {
                emit(ir::addressOf(addrSrc, ptrName));
                addrSrc = ptrName;
            }
        }
        emit(ir::dereference(
                addrSrc, ptrName, expression.result(store_)->getName(),
                type::memoryAccessSizeBytes(resType), type::memoryAccessIsSigned(resType)));
        break;
    }
    case OperatorKind::Add:
        break;
    case OperatorKind::Sub:
        emit(ir::unaryMinus(expression.getOperandExpression()->result(store_)->getName(), expression.result(store_)->getName()));
        narrowIntegralResult(expression.result(store_)->getType(), expression.result(store_)->getName());
        break;
    case OperatorKind::BitNot:
        emit(ir::unaryNot(expression.getOperandExpression()->result(store_)->getName(), expression.result(store_)->getName()));
        narrowIntegralResult(expression.result(store_)->getType(), expression.result(store_)->getName());
        break;
    case OperatorKind::LogicalNot:
        emit(ir::zeroCompare(expression.getOperandExpression()->result(store_)->getName()));
        emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Truthy)->getName(), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant("0", expression.result(store_)->getName()));
        emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
        emit(ir::label(store_.label(&expression, symbols::LabelSlot::Truthy)->getName()));
        emit(ir::assignConstant("1", expression.result(store_)->getName()));
        emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    generateExpression(*expression.getOperandExpression());
    const type::Type& target = expression.result(store_)->getType();
    const type::Type& source = expression.getOperandExpression()->result(store_)->getType();
    // Array-to-pointer cast: (T*)arr must yield &arr[0], not the first word of
    // the array contents (git sha1dc: (const char*)(sha1_padding) where pad[0]==0x80).
    if (source.isArray() && target.isPointer()) {
        if (auto* lvalue = expression.getOperandExpression()->lvalueAnnotation(store_)) {
            // Member / subscript array: address already materialized as lvalue.
            emit(ir::assign(
                    lvalue->getName(), expression.result(store_)->getName()));
        } else {
            emit(ir::addressOf(
                    expression.getOperandExpression()->result(store_)->getName(),
                    expression.result(store_)->getName()));
        }
        return;
    }
    // Assign converts between FLOATING and INTEGRAL when Value types differ (SSE).
    emit(ir::assign(expression.getOperandExpression()->result(store_)->getName(), expression.result(store_)->getName()));
    // Integer casts must narrow: (unsigned char)(-1) is 255, not all-ones.
    // Locals pad sub-word types to 8 bytes, so emit an explicit truncate.
    if (target.kind() == type::TypeKind::Primitive && !type::isFloating(target)) {
        const int size = target.getSize();
        if (size > 0 && size < 8) {
            emit(ir::truncate(
                    expression.result(store_)->getName(),
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
            type::isUnsignedSide(expression.getLeftOperand()->valueType(store_))
            || type::isUnsignedSide(expression.getRightOperand()->valueType(store_));
    const OperatorKind op = expression.getOperator()->getKind();
    const std::string& left = expression.getLeftOperand()->result(store_)->getName();
    const std::string& right = expression.getRightOperand()->result(store_)->getName();
    const std::string& result = expression.result(store_)->getName();
    const symbols::PointerArithPlan* ptrPlan = store_.pointerArithPlan(&expression);

    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        if (const auto* diff = symbols::get_if<symbols::PointerDifferencePlan>(ptrPlan)) {
            // (p - q) / sizeof(*p) -> element count (C 6.5.6).
            emitBinaryOp(*this, OperatorKind::Sub, left, right, result);
            if (diff->scale > 1) {
                emit(ir::assignConstant(
                        std::to_string(diff->scale), diff->scaleTempName));
                emitBinaryOp(*this, OperatorKind::Div, result, diff->scaleTempName, result, false);
            }
        } else if (const auto* scale = symbols::get_if<symbols::PointerScalePlan>(ptrPlan)) {
            // Pointer +/- integer: scaled = index * sizeof(pointee); then ptr +/- scaled.
            const std::string& indexName = scale->pointerOnLeft ? right : left;
            const std::string& ptrName = scale->pointerOnLeft ? left : right;
            emit(ir::assignConstant(
                    std::to_string(scale->scale), scale->scaleTempName));
            emitBinaryOp(*this, OperatorKind::Mul, indexName, scale->scaleTempName, scale->scaleTempName);
            emitBinaryOp(*this, op, ptrName, scale->scaleTempName, result);
        } else {
            emitBinaryOp(*this, op, left, right, result);
        }
    } else if (op == OperatorKind::Mul || op == OperatorKind::Div || op == OperatorKind::Mod) {
        emitBinaryOp(*this, op, left, right, result, unsignedDiv);
    } else {
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
    // Pointer results stay full-width; integral sub-word results need re-extension.
    if (!expression.result(store_)->getType().isPointer()) {
        narrowIntegralResult(expression.result(store_)->getType(), result);
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const OperatorKind op = expression.getOperator()->getKind();
    // C 6.5.7: unsigned >> is logical (zero-fill); signed is arithmetic (SAR).
    const type::Type shiftedType = expression.valueType(store_);
    const bool logical = type::isIntegral(shiftedType) && !type::valueIsSigned(shiftedType);
    if (op != OperatorKind::Shl && op != OperatorKind::Shr) {
        throw std::runtime_error { "unidentified shift operator!" };
    }
    emitBinaryOp(*this, op,
            expression.getLeftOperand()->result(store_)->getName(),
            expression.getRightOperand()->result(store_)->getName(),
            expression.result(store_)->getName(),
            false, logical);
    narrowIntegralResult(expression.result(store_)->getType(), expression.result(store_)->getName());
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
            type::isUnsignedSide(expression.getLeftOperand()->valueType(store_))
            || type::isUnsignedSide(expression.getRightOperand()->valueType(store_));

    // Mixed signed/unsigned equality must convert both to the common unsigned
    // width first. Otherwise unsigned int 0xffffffff (zero-ext in a 64-bit reg)
    // does not equal signed -1 (0xffffffffffffffff) - git: opt.sign == -1.
    if (unsignedCompare) {
        const int leftW = typeWidth(expression.getLeftOperand()->valueType(store_));
        const int rightW = typeWidth(expression.getRightOperand()->valueType(store_));
        const int common = leftW > rightW ? leftW : rightW;
        if (common > 0 && common < 8) {
            emit(ir::truncate(
                    expression.getLeftOperand()->result(store_)->getName(), common, false));
            emit(ir::truncate(
                    expression.getRightOperand()->result(store_)->getName(), common, false));
        }
    }

    emit(ir::valueCompare(expression.getLeftOperand()->result(store_)->getName(), expression.getRightOperand()->result(store_)->getName()));

    auto truthyLabel = store_.label(&expression, symbols::LabelSlot::Truthy)->getName();
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
    emit(ir::jump(truthyLabel, cond));

    emit(ir::assignConstant("0", expression.result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant("1", expression.result(store_)->getName()));
    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const ast::OperatorKind op = expression.getOperator()->getKind();
    if (op != ast::OperatorKind::BitAnd && op != ast::OperatorKind::BitOr && op != ast::OperatorKind::BitXor) {
        throw std::runtime_error { "no semantic actions defined for bitwise operator: "
                + expression.getOperator()->getLexeme() };
    }
    emitBinaryOp(*this, op,
            expression.getLeftOperand()->result(store_)->getName(),
            expression.getRightOperand()->result(store_)->getName(),
            expression.result(store_)->getName());
    narrowIntegralResult(expression.result(store_)->getType(), expression.result(store_)->getName());
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    emit(ir::assignConstant("0", expression.result(store_)->getName()));
    emit(ir::zeroCompare(expression.getLeftOperand()->result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_EQUAL));

    generateExpression(*expression.getRightOperand());

    emit(ir::zeroCompare(expression.getRightOperand()->result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant("1", expression.result(store_)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    emit(ir::assignConstant("1", expression.result(store_)->getName()));
    emit(ir::zeroCompare(expression.getLeftOperand()->result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_NOT_EQUAL));

    generateExpression(*expression.getRightOperand());

    emit(ir::zeroCompare(expression.getRightOperand()->result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", expression.result(store_)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    generateExpression(*expression.getCondition());
    emit(ir::zeroCompare(expression.getCondition()->result(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName(), JumpCondition::IF_EQUAL));

    const bool producesValue = expression.hasResult(store_)
            && !expression.result(store_)->getType().isVoid();

    std::string trueName = generateExpression(*expression.getTrueExpression());
    if (producesValue) {
        emit(ir::assign(
                trueName, expression.result(store_)->getName()));
    }
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
    std::string falseName = generateExpression(*expression.getFalseExpression());
    if (producesValue) {
        emit(ir::assign(
                falseName, expression.result(store_)->getName()));
    }

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    generateExpression(*expression.getRightOperand());

    const OperatorKind assignKind = expression.getOperator()->getKind();
    auto resultName = expression.result(store_)->getName();

    if (assignKind == OperatorKind::Assign) {
        if (expression.getLeftOperand()->lvalueAnnotation(store_)) {
            // Convert into the assignment result temp first, then store.
            // LvalueAssign alone bitcasts; float->int must cvttsd2si before the
            // memory write (git: d->rename_score = p->score * 100 / MAX_SCORE).
            // C 6.5.16: result is the value stored - left operand result still holds
            // the pre-store load; refresh it so (p = malloc(n), p) sees the new
            // pointer (git DUP_ARRAY in copy_pathspec).
            emit(ir::assign(
                        expression.getRightOperand()->result(store_)->getName(),
                        resultName
            ));
            emit(ir::lvalueAssign(
                        resultName,
                        expression.getLeftOperand()->lvalueAnnotation(store_)->getName(),
                        type::memoryAccessSizeBytes(expression.getLeftOperand()->valueType(store_))
            ));
        } else {
            emit(ir::assign(
                        expression.getRightOperand()->result(store_)->getName(),
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

    // Pointer compound assign: scale integer RHS by pointee size (PointerScalePlan).
    std::string rhsName = expression.getRightOperand()->result(store_)->getName();
    if ((baseOp == OperatorKind::Add || baseOp == OperatorKind::Sub)) {
        if (const auto* scale = symbols::get_if<symbols::PointerScalePlan>(
                    store_.pointerArithPlan(&expression))) {
            emit(ir::assignConstant(
                    std::to_string(scale->scale), scale->scaleTempName));
            emitBinaryOp(*this, OperatorKind::Mul, rhsName, scale->scaleTempName, scale->scaleTempName);
            rhsName = scale->scaleTempName;
        }
    }

    const bool unsignedDiv =
            type::isUnsignedSide(expression.getLeftOperand()->valueType(store_))
            || type::isUnsignedSide(expression.getRightOperand()->valueType(store_));
    const type::Type lhsType = expression.getLeftOperand()->valueType(store_);
    const bool logicalShr = type::isIntegral(lhsType) && !type::valueIsSigned(lhsType);
    emitBinaryOp(*this, baseOp, resultName, rhsName, resultName, unsignedDiv, logicalShr);

    // Compound assign is a read-modify-write of a C type width (often < 8).
    // Without truncate, e.g. uint32_t x = 0xffffffff; x += 1 leaves bit 32 set
    // in the register, which poisons later shifts/rotates (git sha1dc e += ...).
    if (!expression.result(store_)->getType().isPointer()) {
        narrowIntegralResult(expression.result(store_)->getType(), resultName);
    }

    // Compound assign updated the value temp; write back through pointer lvalues (e.g. *p += 1).
    if (auto* lvalue = expression.getLeftOperand()->lvalueAnnotation(store_)) {
        emit(ir::lvalueAssign(
                resultName, lvalue->getName(), type::memoryAccessSizeBytes(expression.getLeftOperand()->valueType(store_))));
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
    const auto* field = symbols::get_if<symbols::FieldPlan>(store_.addressPlan(&expression));
    if (!field) {
        throw std::runtime_error { "MemberAccess missing FieldPlan from SA" };
    }
    const std::string addrName = expression.lvalueAnnotation(store_)->getName();
    const std::string resultName = expression.result(store_)->getName();
    emitFieldFromPlan(*field, addrName, *this);
    emitAddressResultOrLoad(*this, addrName, resultName, expression.holdsAggregateAddress(),
            expression.valueType(store_));
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    generateExpression(*arrayAccess.getLeftOperand());
    generateExpression(*arrayAccess.getRightOperand());

    // Without SA (or on SA error), no lvalue/result temps - no IR.
    if (!arrayAccess.lvalueAnnotation(store_) || !arrayAccess.result(store_)) {
        return;
    }
    const auto* idx = symbols::get_if<symbols::IndexPlan>(store_.addressPlan(&arrayAccess));
    if (!idx) {
        return;
    }
    const std::string addrName = arrayAccess.lvalueAnnotation(store_)->getName();
    const std::string resultName = arrayAccess.result(store_)->getName();
    emitIndexFromPlan(*idx, arrayAccess.getRightOperand()->result(store_)->getName(),
            addrName, *this);
    emitAddressResultOrLoad(*this, addrName, resultName, arrayAccess.holdsAggregateAddress(),
            arrayAccess.result(store_)->getType());
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    // Nested brace lists (array of structs) must evaluate leaf expressions so
    // StructFieldInit sources hold values before LvalueAssign (git options[]).
    expression.visitElements(*this);
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteralExpression& expression) {
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }
    if (!store_.value(&expression, symbols::ValueSlot::Object)) {
        return;
    }
    emitStructFieldInits(*this, store_.value(&expression, symbols::ValueSlot::Object)->getName(),
            store_.structFieldInits(&expression));
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (identifier.hasFoldedConstant()) {
        emit(ir::assignConstant(
                std::to_string(identifier.getFoldedConstant()), identifier.result(store_)->getName()));
    } else if (identifier.holdsFunctionDesignator()) {
        // Load address of function into the result temporary (name on FunctionDesignatorPlan).
        const std::string* name = identifier.functionDesignatorName(store_);
        if (!name) {
            throw std::runtime_error { "FunctionDesignator form missing FunctionDesignatorPlan name" };
        }
        emit(ir::functionAddress(
                *name, identifier.result(store_)->getName()));
    }
}


void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so C suffixes (ul/ULL) and char escapes
    // never reach NASM as raw lexemes.
    std::string immediate;
    if (type::isFloating(constant.expressionType())) {
        // Keep full precision as IEEE bits; evaluateConstant would truncate.
        if (!util::floatingLiteralImmediate(constant.getValue(), immediate)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
    } else {
        long value;
        if (constant.evaluateConstant(value)) {
            // Prefer hex for values outside signed 32-bit so NASM does not warn
            // "signed dword exceeds bounds" on 64-bit immediates.
            immediate = util::wordImmediate(static_cast<unsigned long long>(value));
        } else {
            immediate = constant.getValue();
            while (!immediate.empty()) {
                char c = immediate.back();
                if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                    immediate.pop_back();
                } else {
                    break;
                }
            }
        }
    }
    emit(ir::assignConstant(immediate, constant.result(store_)->getName()));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    // Pool labels are addresses: AssignLabelAddress -> lea (PIE-safe), not mov imm.
    emit(ir::assignLabelAddress(
            stringLiteral.getConstantSymbol(), stringLiteral.result(store_)->getName()));
}



} // namespace codegen
