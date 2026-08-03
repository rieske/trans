#include "CodeGeneratingVisitor.h"
#include "CodeGeneratingVisitorInternal.h"
#include "symbols/AddressPlan.h"
#include "Instruction.h"
#include "ast/ArrayAccess.h"

#include <stdexcept>
#include <vector>

#include "symbols/ValueEntry.h"
#include "symbols/LabelEntry.h"
#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"

#include "ast/MemberAccess.h"
#include "ast/StatementExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/Expression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/CompoundLiteralExpression.h"
#include "ast/GenericSelection.h"

namespace codegen {

void CodeGeneratingVisitor::emitFloatingConstant(const std::string& dest,
        const util::FloatingBits& bits) {
    const std::string lo = util::hexImmediate(bits.bits);
    if (bits.sizeBytes > 8) {
        emit(ir::assignConstant(lo, util::hexImmediate(bits.bitsHi), dest));
    } else {
        emit(ir::assignConstant(lo, dest));
    }
}

// Integer/pointer: ir::inc/dec with pointee step. Floating: add/sub 1.0 (real ++/--).
// Pointer-to-VLA steps by runtime sizeof of the pointee.
void CodeGeneratingVisitor::emitIncDec(const std::string& name, const type::Type& valueType,
        bool increment) {
    if (type::isFloating(valueType)) {
        const std::string one = addScratchValue(valueType);
        emitFloatingConstant(one, util::floatingOne(valueType.getSize()));
        if (increment) {
            emit(ir::add(name, one, name));
        } else {
            emit(ir::sub(name, one, name));
        }
        return;
    }
    if (valueType.isPointer()) {
        const std::string one = addScratchValue(type::signedInteger());
        emit(ir::assignConstant("1", one));
        const ScaledIndex scaled = scaleIndex(
                valueType.dereference(), one, type::pointerElementStride(valueType));
        if (scaled.name == one) {
            if (increment) {
                emit(ir::inc(name, scaled.strideBytes));
            } else {
                emit(ir::dec(name, scaled.strideBytes));
            }
        } else if (increment) {
            emit(ir::add(name, scaled.name, name));
        } else {
            emit(ir::sub(name, scaled.name, name));
        }
        return;
    }
    if (increment) {
        emit(ir::inc(name, 1));
    } else {
        emit(ir::dec(name, 1));
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto* operand = expression.getOperandExpression();
    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    auto valueName = operand->getResultSymbol(store_)->getName();
    emit(ir::assign(valueName, resultSymbolName));

    const bool increment = expression.getOperator()->getKind() == ast::OperatorKind::Inc;
    emitIncDec(valueName, operand->valueType(store_), increment);

    if (operand->getLvalueSymbol(store_)) {
        emitLvalueStore(*operand, valueName);
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    generateExpression(*expression.getOperandExpression());

    auto resultSymbolName = expression.getResultSymbol(store_)->getName();
    const bool increment = expression.getOperator()->getKind() == ast::OperatorKind::Inc;
    emitIncDec(resultSymbolName, expression.getOperandExpression()->valueType(store_), increment);

    if (expression.getOperandExpression()->getLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }
}

void CodeGeneratingVisitor::visit(ast::TypeNameExpression&) {
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    using ast::OperatorKind;
    const OperatorKind op = expression.getOperator()->getKind();
    if (expression.hasFoldedInteger()) {
        // sizeof result or offsetof (&((T*)0)->m) - both are SA-folded integers.
        emit(ir::assignConstant(
                std::to_string(expression.foldedInteger()), expression.getResultSymbol(store_)->getName()));
        return;
    }
    if (expression.isSizeof()) {
        emitSizeofProduct(expression.operandType(),
                expression.getResultSymbol(store_)->getName());
        return;
    }

    if (op == OperatorKind::AddressOf) {
        emitAddressOf(*expression.getOperandExpression(), expression.getResultSymbol(store_)->getName());
        return;
    }

    const std::string operandName = generateExpression(*expression.getOperandExpression());

    switch (op) {
    case OperatorKind::Deref: {
        auto* operand = expression.getOperandExpression();
        auto* lv = expression.getLvalueSymbol(store_);
        auto* op = operand->getResultSymbol(store_);
        if (lv && !sameValueTemp(lv, op)) {
            emitArrayObjectAddress(*op, lv->getName());
        }
        if (expression.holdsAggregateAddress()) {
            break;
        }
        // *fp: SA aliases the operand pointer temp (no memory load).
        if (sameValueTemp(expression.getResultSymbol(store_), op)) {
            break;
        }
        if (lv) {
            emit(ir::dereference(lv->getName(), expression.getResultSymbol(store_)->getName()));
        }
        break;
    }
    case OperatorKind::Add:
        emit(ir::assign(operandName, expression.getResultSymbol(store_)->getName()));
        break;
    case OperatorKind::Sub:
        emit(ir::unaryMinus(operandName, expression.getResultSymbol(store_)->getName()));
        narrowIntegralResult(expression.getResultSymbol(store_)->getType(), expression.getResultSymbol(store_)->getName());
        break;
    case OperatorKind::BitNot:
        emit(ir::unaryNot(operandName, expression.getResultSymbol(store_)->getName()));
        narrowIntegralResult(expression.getResultSymbol(store_)->getType(), expression.getResultSymbol(store_)->getName());
        break;
    case OperatorKind::LogicalNot:
        emit(ir::zeroCompare(expression.getOperandExpression()->getResultSymbol(store_)->getName()));
        emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Truthy)->getName(), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
        emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
        emit(ir::label(store_.label(&expression, symbols::LabelSlot::Truthy)->getName()));
        emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
        emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::GenericSelection& expression) {
    if (!expression.hasSelected()) {
        return;
    }
    expression.selectedExpression().accept(*this);
}

void CodeGeneratingVisitor::visit(ast::StatementExpression& expression) {
    expression.body().accept(*this);
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    generateExpression(*expression.getOperandExpression());
    const type::Type& target = expression.getResultSymbol(store_)->getType();
    const type::Type& source = expression.getOperandExpression()->getResultSymbol(store_)->getType();
    // Array-to-pointer cast: (T*)arr must yield &arr[0], not the first word of
    // the array contents (git sha1dc: (const char*)(sha1_padding) where pad[0]==0x80).
    if (source.isArray() && target.isPointer()) {
        if (auto* lvalue = expression.getOperandExpression()->getLvalueSymbol(store_)) {
            // Member / subscript array: address already materialized as lvalue.
            emit(ir::assign(
                    lvalue->getName(), expression.getResultSymbol(store_)->getName()));
        } else {
            emit(ir::addressOf(
                    expression.getOperandExpression()->getResultSymbol(store_)->getName(),
                    expression.getResultSymbol(store_)->getName()));
        }
        return;
    }
    // Assign converts between FLOATING and INTEGRAL when Value types differ (SSE).
    // Bool dest is 0/1 (C 6.3.1.2), not a bitcast of the source word.
    emitConvert(expression.getOperandExpression()->getResultSymbol(store_)->getName(),
            expression.getResultSymbol(store_)->getName(), source, target);
    // Integer casts must narrow: (unsigned char)(-1) is 255, not all-ones.
    // Locals pad sub-word types to 8 bytes, so emit an explicit truncate.
    if (target.kind() == type::TypeKind::Primitive && !type::isFloating(target)) {
        const int size = target.getSize();
        if (size > 0 && size < 8) {
            emit(ir::truncate(
                    expression.getResultSymbol(store_)->getName(),
                    size,
                    type::valueIsSigned(target)));
        }
    }
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    using ast::OperatorKind;
    const std::string left = generateExpression(*expression.getLeftOperand());
    const std::string right = generateExpression(*expression.getRightOperand());

    // C usual arithmetic conversions: if either side is unsigned, / and % are unsigned.
    // Signed idiv traps when quotient has high bit set (git st_mult: SIZE_MAX / a).
    // Value types after decay (not expressionType: member arrays stay T[N] there).
    const bool unsignedDiv =
            type::isUnsignedSide(expression.getLeftOperand()->valueType(store_))
            || type::isUnsignedSide(expression.getRightOperand()->valueType(store_));
    const OperatorKind op = expression.getOperator()->getKind();
    const std::string& result = expression.getResultSymbol(store_)->getName();
    const symbols::PointerArithPlan* ptrPlan = store_.pointerArithPlan(&expression);

    if (op == OperatorKind::Add || op == OperatorKind::Sub) {
        if (const auto* diff = symbols::get_if<symbols::PointerDifferencePlan>(ptrPlan)) {
            // (p - q) / sizeof(*p) -> element count (C 6.5.6).
            emitBinaryOp(*this, OperatorKind::Sub, left, right, result);
            const type::Type pointee = expression.getLeftOperand()->valueType(store_).dereference();
            if (type::hasComputableRuntimeSize(pointee)) {
                const std::string size = addScratchValue(type::signedInteger());
                emitSizeofProduct(pointee, size);
                emitBinaryOp(*this, OperatorKind::Div, result, size, result, false);
            } else if (diff->scale > 1) {
                emit(ir::assignConstant(
                        std::to_string(diff->scale), diff->scaleTempName));
                emitBinaryOp(*this, OperatorKind::Div, result, diff->scaleTempName, result, false);
            }
        } else if (const auto* scale = symbols::get_if<symbols::PointerScalePlan>(ptrPlan)) {
            // Pointer +/- integer: scaled = index * sizeof(pointee); then ptr +/- scaled.
            const std::string& indexName = scale->pointerOnLeft ? right : left;
            const std::string& ptrName = scale->pointerOnLeft ? left : right;
            const type::Type pointerType = scale->pointerOnLeft
                    ? expression.getLeftOperand()->valueType(store_)
                    : expression.getRightOperand()->valueType(store_);
            const ScaledIndex scaled = scaleIndex(pointerType.dereference(), indexName, scale->scale);
            if (scaled.name == indexName && scale->scale > 1) {
                emit(ir::assignConstant(
                        std::to_string(scale->scale), scale->scaleTempName));
                emitBinaryOp(*this, OperatorKind::Mul, indexName, scale->scaleTempName, scale->scaleTempName);
                emitBinaryOp(*this, op, ptrName, scale->scaleTempName, result);
            } else {
                emitBinaryOp(*this, op, ptrName, scaled.name, result);
            }
        } else {
            emitBinaryOp(*this, op, left, right, result);
        }
    } else if (op == OperatorKind::Mul || op == OperatorKind::Div || op == OperatorKind::Mod) {
        emitMulDiv(op, left, right, result,
                expression.getResultSymbol(store_)->getType(), unsignedDiv);
    } else {
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
    // Pointer results stay full-width; integral sub-word results need re-extension.
    if (!expression.getResultSymbol(store_)->getType().isPointer()) {
        narrowIntegralResult(expression.getResultSymbol(store_)->getType(), result);
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    using ast::OperatorKind;
    const std::string left = generateExpression(*expression.getLeftOperand());
    const std::string right = generateExpression(*expression.getRightOperand());

    const OperatorKind op = expression.getOperator()->getKind();
    // C 6.5.7: unsigned >> is logical (zero-fill); signed is arithmetic (SAR).
    const type::Type shiftedType = expression.valueType(store_);
    const bool logical = type::isIntegral(shiftedType) && !type::valueIsSigned(shiftedType);
    if (op != OperatorKind::Shl && op != OperatorKind::Shr) {
        throw std::runtime_error { "unidentified shift operator!" };
    }
    emitBinaryOp(*this, op, left, right, expression.getResultSymbol(store_)->getName(), false, logical);
    narrowIntegralResult(expression.getResultSymbol(store_)->getType(), expression.getResultSymbol(store_)->getName());
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    const std::string leftName = generateExpression(*expression.getLeftOperand());
    const std::string rightName = generateExpression(*expression.getRightOperand());

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
    const type::Type leftTy = expression.getLeftOperand()->valueType(store_);
    const type::Type rightTy = expression.getRightOperand()->valueType(store_);
    // ucomi sets CF/ZF like unsigned cmp; signed jl/jg is wrong for two negatives.
    const bool floatingCompare = type::isFloating(leftTy) || type::isFloating(rightTy);
    const bool unsignedCompare = floatingCompare
            || type::isUnsignedSide(leftTy) || type::isUnsignedSide(rightTy);

    // Mixed signed/unsigned equality must convert both to the common unsigned
    // width first. Otherwise unsigned int 0xffffffff (zero-ext in a 64-bit reg)
    // does not equal signed -1 (0xffffffffffffffff) - git: opt.sign == -1.
    if (unsignedCompare && !floatingCompare) {
        const int leftW = typeWidth(expression.getLeftOperand()->valueType(store_));
        const int rightW = typeWidth(expression.getRightOperand()->valueType(store_));
        const int common = leftW > rightW ? leftW : rightW;
        if (common > 0 && common < 8) {
            emit(ir::truncate(leftName, common, false));
            emit(ir::truncate(rightName, common, false));
        }
    }

    emit(ir::valueCompare(leftName, rightName, !unsignedCompare));

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

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    const std::string left = generateExpression(*expression.getLeftOperand());
    const std::string right = generateExpression(*expression.getRightOperand());

    const ast::OperatorKind op = expression.getOperator()->getKind();
    if (op != ast::OperatorKind::BitAnd && op != ast::OperatorKind::BitOr && op != ast::OperatorKind::BitXor) {
        throw std::runtime_error { "no semantic actions defined for bitwise operator: "
                + expression.getOperator()->getLexeme() };
    }
    emitBinaryOp(*this, op, left, right, expression.getResultSymbol(store_)->getName());
    narrowIntegralResult(expression.getResultSymbol(store_)->getType(), expression.getResultSymbol(store_)->getName());
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.getLeftOperand()->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_EQUAL));

    generateExpression(*expression.getRightOperand());

    emit(ir::zeroCompare(expression.getRightOperand()->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    generateExpression(*expression.getLeftOperand());

    emit(ir::assignConstant("1", expression.getResultSymbol(store_)->getName()));
    emit(ir::zeroCompare(expression.getLeftOperand()->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_NOT_EQUAL));

    generateExpression(*expression.getRightOperand());

    emit(ir::zeroCompare(expression.getRightOperand()->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName(), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant("0", expression.getResultSymbol(store_)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    generateExpression(*expression.getCondition());
    emit(ir::zeroCompare(expression.getCondition()->getResultSymbol(store_)->getName()));
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Falsy)->getName(), JumpCondition::IF_EQUAL));

    const bool producesValue = expression.hasResultSymbol(store_)
            && !expression.getResultSymbol(store_)->getType().isVoid();

    std::string trueName = generateExpression(*expression.getTrueExpression());
    if (producesValue) {
        emit(ir::assign(
                trueName, expression.getResultSymbol(store_)->getName()));
    }
    emit(ir::jump(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Falsy)->getName()));
    std::string falseName = generateExpression(*expression.getFalseExpression());
    if (producesValue) {
        emit(ir::assign(
                falseName, expression.getResultSymbol(store_)->getName()));
    }

    emit(ir::label(store_.label(&expression, symbols::LabelSlot::Exit)->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    using ast::OperatorKind;
    generateExpression(*expression.getLeftOperand());
    std::string rhsName = generateExpression(*expression.getRightOperand());

    const OperatorKind assignKind = expression.getOperator()->getKind();
    auto resultName = expression.getResultSymbol(store_)->getName();

    if (assignKind == OperatorKind::Assign) {
        if (expression.getLeftOperand()->getLvalueSymbol(store_)) {
            // Convert into the assignment result temp first, then store.
            // LvalueAssign alone bitcasts; float->int must cvttsd2si before the
            // memory write (git: d->rename_score = p->score * 100 / MAX_SCORE).
            // C 6.5.16: result is the value stored - left operand result still holds
            // the pre-store load; refresh it so (p = malloc(n), p) sees the new
            // pointer (git DUP_ARRAY in copy_pathspec).
            emit(ir::assign(rhsName, resultName));
            emitLvalueStore(*expression.getLeftOperand(), resultName);
        } else {
            emit(ir::assign(rhsName, resultName));
        }
        return;
    }

    const OperatorKind baseOp = ast::compoundAssignBase(assignKind);
    if (baseOp == OperatorKind::Unknown) {
        throw std::runtime_error { "unidentified assignment operator: "
                + expression.getOperator()->getLexeme() };
    }

    // Pointer compound assign: scale integer RHS by pointee size (PointerScalePlan).
    if ((baseOp == OperatorKind::Add || baseOp == OperatorKind::Sub)) {
        if (const auto* scale = symbols::get_if<symbols::PointerScalePlan>(
                    store_.pointerArithPlan(&expression))) {
            const type::Type lhsType = expression.getLeftOperand()->valueType(store_);
            const ScaledIndex scaled = scaleIndex(lhsType.dereference(), rhsName, scale->scale);
            if (scaled.name == rhsName && scale->scale > 1) {
                emit(ir::assignConstant(
                        std::to_string(scale->scale), scale->scaleTempName));
                emitBinaryOp(*this, OperatorKind::Mul, rhsName, scale->scaleTempName, scale->scaleTempName);
                rhsName = scale->scaleTempName;
            } else {
                rhsName = scaled.name;
            }
        }
    }

    const bool unsignedDiv =
            type::isUnsignedSide(expression.getLeftOperand()->valueType(store_))
            || type::isUnsignedSide(expression.getRightOperand()->valueType(store_));
    const type::Type lhsType = expression.getLeftOperand()->valueType(store_);
    const bool logicalShr = type::isIntegral(lhsType) && !type::valueIsSigned(lhsType);
    if (baseOp == OperatorKind::Mul || baseOp == OperatorKind::Div || baseOp == OperatorKind::Mod) {
        emitMulDiv(baseOp, resultName, rhsName, resultName,
                expression.getResultSymbol(store_)->getType(), unsignedDiv);
    } else {
        emitBinaryOp(*this, baseOp, resultName, rhsName, resultName, unsignedDiv, logicalShr);
    }

    // Compound assign is a read-modify-write of a C type width (often < 8).
    // Without truncate, e.g. uint32_t x = 0xffffffff; x += 1 leaves bit 32 set
    // in the register, which poisons later shifts/rotates (git sha1dc e += ...).
    if (!expression.getResultSymbol(store_)->getType().isPointer()) {
        narrowIntegralResult(expression.getResultSymbol(store_)->getType(), resultName);
    }

    // Compound assign updated the value temp; write back through pointer lvalues (e.g. *p += 1).
    if (expression.getLeftOperand()->getLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getLeftOperand(), resultName);
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
    emitFieldFromPlan(*field, *expression.getBase(),
            expression.getLvalueSymbol(store_)->getName(), *this, store_);
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    generateExpression(*arrayAccess.getLeftOperand());
    generateExpression(*arrayAccess.getRightOperand());

    if (!arrayAccess.hasResultSymbol(store_)) {
        return;
    }
    const auto* idx = symbols::get_if<symbols::IndexPlan>(store_.addressPlan(&arrayAccess));
    if (!idx) {
        throw std::runtime_error { "ArrayAccess missing IndexPlan from SA" };
    }
    if (!arrayAccess.getLvalueSymbol(store_)) {
        throw std::runtime_error { "ArrayAccess missing lvalue from SA" };
    }
    ast::Expression& baseExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(), idx->baseOperand);
    ast::Expression& indexExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(),
            symbols::otherBinaryOperand(idx->baseOperand));
    emitIndexFromPlan(*idx, baseExpr,
            indexExpr.getResultSymbol(store_)->getName(),
            arrayAccess.getLvalueSymbol(store_)->getName(), *this, store_);
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    // Nested brace lists (array of structs) must evaluate leaf expressions so
    // FieldInit sources hold values before LvalueAssign (git options[]).
    expression.visitElements(*this);
    // FieldPlanSink names Conversion temps; emit that IR here so stores see filled temps.
    for (const auto& element : expression.getElements()) {
        if (element.value && element.value->hasResultSymbol(store_)) {
            materializeFieldIndexLoad(*element.value, *this, store_);
            materializeConversion(*element.value);
        }
    }
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteralExpression& expression) {
    if (expression.getInitializer()) {
        expression.getInitializer()->accept(*this);
    }
    if (!store_.value(&expression, symbols::ValueSlot::Object)) {
        return;
    }
    emitFieldInits(*this, store_.value(&expression, symbols::ValueSlot::Object)->getName(),
            store_.fieldInits(&expression));
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (const std::string* label = store_.string(&identifier, symbols::StringSlot::ConstantLabel)) {
        emit(ir::assignLabelAddress(
                *label, identifier.getResultSymbol(store_)->getName()));
        return;
    }
    type::IntegerConstant ice;
    if (identifier.evaluateConstant(ice)) {
        emitIntegerConstant(ice, identifier.getResultSymbol(store_)->getName());
    } else if (identifier.holdsFunctionDesignator()) {
        const auto* plan = store_.addressPlan(&identifier);
        const auto* designator = symbols::get_if<symbols::FunctionDesignatorPlan>(plan);
        if (!designator || !designator->functionName) {
            throw std::runtime_error { "FunctionDesignator form missing FunctionDesignatorPlan name" };
        }
        emit(ir::functionAddress(
                *designator->functionName, identifier.getResultSymbol(store_)->getName()));
    }
}


void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so C suffixes (ul/ULL) and char escapes
    // never reach NASM as raw lexemes.
    const std::string resultName = constant.getResultSymbol(store_)->getName();
    if (type::isFloating(constant.expressionType())) {
        util::FloatingBits parsed;
        if (!util::floatingLiteralBits(constant.getValue(), parsed)) {
            throw std::runtime_error { "invalid floating constant: " + constant.getValue() };
        }
        emitFloatingConstant(resultName, parsed);
        return;
    }
    type::IntegerConstant value;
    if (!constant.evaluateConstant(value)) {
        throw std::runtime_error { "invalid integer constant: " + constant.getValue() };
    }
    emitIntegerConstant(value, resultName);
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    // Pool labels are addresses: AssignLabelAddress -> lea (PIE-safe), not mov imm.
    emit(ir::assignLabelAddress(
            stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol(store_)->getName()));
}

void CodeGeneratingVisitor::emitLvalueStore(ast::Expression& lhs, const std::string& valueName) {
    auto* lvalue = lhs.getLvalueSymbol(store_);
    if (!lvalue) {
        return;
    }
    if (const auto* bits = symbols::bitFieldOf(store_.addressPlan(&lhs))) {
        emitBitFieldInsert(lvalue->getName(), valueName, *bits, lhs.valueType(store_));
        return;
    }
    emit(ir::lvalueAssign(valueName, lvalue->getName(),
            type::memoryAccessSizeBytes(lhs.valueType(store_))));
}

} // namespace codegen
