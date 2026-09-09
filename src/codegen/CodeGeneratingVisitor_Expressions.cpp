#include "CodeGeneratingVisitor.h"
#include "ast/AstNodes.h"
#include "codegen/InternalError.h"
#include "codegen/IrBuilders.h"

#include <cassert>
#include <stdexcept>

#include "symbols/AddressPlan.h"
#include "symbols/ValueEntry.h"
#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"
#include "util/ImmediateFormat.h"

namespace codegen {

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);
    if (!arrayAccess.getLvalueSymbol(store_) || !arrayAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* indexPlan = store_.addressPlan(&arrayAccess);
    const auto* index = indexPlan ? symbols::get_if<symbols::IndexPlan>(indexPlan) : nullptr;
    require(index, "IndexPlan for array codegen");
    ast::Expression& baseExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(), index->baseOperand);
    ast::Expression& indexExpr = symbols::pickBinaryOperand(
            *arrayAccess.getLeftOperand(), *arrayAccess.getRightOperand(),
            symbols::otherBinaryOperand(index->baseOperand));
    const ScaledIndex scaled = scaleIndex(
            index->elementType,
            id(*indexExpr.getResultSymbol(store_)),
            index->elementSize);
    emit(ir::indexAddress(
            id(*baseExpr.getResultSymbol(store_)),
            scaled.name,
            scaled.strideBytes,
            id(*arrayAccess.getLvalueSymbol(store_)),
            index->baseMode));
    if (!arrayAccess.holdsAggregateAddress()) {
        const int addr = id(*arrayAccess.getLvalueSymbol(store_));
        emit(ir::dereference(addr, addr, id(*arrayAccess.getResultSymbol(store_))));
    }
}

void CodeGeneratingVisitor::visit(ast::InitializerListExpression& expression) {
    expression.visitElements(*this);
    // FieldPlanSink names Conversion temps; emit that IR here so stores see filled temps.
    for (const auto& element : expression.getElements()) {
        if (element.value && element.value->hasResultSymbol(store_)) {
            convertedResult(*element.value);
        }
    }
}

void CodeGeneratingVisitor::visit(ast::MemberAccess& memberAccess) {
    memberAccess.getBase()->accept(*this);
    if (!memberAccess.getLvalueSymbol(store_) || !memberAccess.getResultSymbol(store_)) {
        return;
    }
    const auto* plan = store_.addressPlan(&memberAccess);
    const auto* field = plan ? symbols::get_if<symbols::FieldPlan>(plan) : nullptr;
    require(field, "FieldPlan for member access codegen");
    const symbols::ValueEntry* baseSym = memberAccess.getBase()->addressSymbol(store_);
    require(baseSym, "member base symbol");
    const int addrTemp = id(*memberAccess.getLvalueSymbol(store_));
    const auto baseMode = baseSym->getType().isPointer()
            ? symbols::AddressBaseMode::PointerValue
            : symbols::AddressBaseMode::LeaObject;
    emit(ir::fieldAddress(
            id(*baseSym),
            field->fieldOffsetBytes,
            addrTemp,
            baseMode));
    if (!memberAccess.holdsAggregateAddress()) {
        const int resultName = id(*memberAccess.getResultSymbol(store_));
        emit(ir::dereference(addrTemp, addrTemp, resultName));
        if (field->isBitField()) {
            emitBitFieldExtract(resultName, resultName, *field->bitField);
        }
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression& identifier) {
    if (const auto* label = identifier.rodataLabel(store_)) {
        assert(identifier.hasResultSymbol(store_) && "__func__ needs Result temp");
        emit(ir::assignLabelAddress(
                id(*label), id(*identifier.getResultSymbol(store_))));
        return;
    }
    type::IntegerConstant ice;
    if (identifier.evaluateConstant(ice)) {
        assert(identifier.hasResultSymbol(store_) && "folded enumerator needs Result temp");
        emitIntegerConstant(ice, id(*identifier.getResultSymbol(store_)));
        return;
    }
    // Function designators: plan holds the label; Result is the address temp.
    if (const auto* plan = store_.addressPlan(&identifier)) {
        if (const auto* d = symbols::get_if<symbols::FunctionDesignatorPlan>(plan)) {
            if (d->functionName) {
                assert(identifier.hasResultSymbol(store_) && "designator Result required for FunctionAddress");
                emit(ir::functionAddress(
                        id(*d->functionName), id(*identifier.getResultSymbol(store_))));
                return;
            }
        }
    }
    if (identifier.holdsFunctionDesignator()) {
        codegen::internalError("designator form without FunctionDesignatorPlan on the store");
    }
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    // Decode to a numeric immediate so suffixes never reach the assembler raw.
    const int resultName = id(*constant.getResultSymbol(store_));
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
    const auto* label = stringLiteral.rodataLabel(store_);
    require(label, "string literal rodata label");
    emit(ir::assignLabelAddress(id(*label), id(*stringLiteral.getResultSymbol(store_))));
}

void CodeGeneratingVisitor::emitIntegerConstant(const type::IntegerConstant& value,
        int dest) {
    const int lo = id(util::wordImmediate(type::bitsWord(value, 0)));
    if (type::object_abi::valueWords(value.type.getSize()) > 1) {
        emit(ir::assignConstant(lo, id(util::wordImmediate(type::bitsWord(value, 1))), dest));
        return;
    }
    emit(ir::assignConstant(lo, dest));
}

void CodeGeneratingVisitor::emitFloatingConstant(int dest, const util::FloatingBits& bits) {
    const int lo = id(util::hexImmediate(bits.bits));
    if (bits.sizeBytes > 8) {
        emit(ir::assignConstant(lo, id(util::hexImmediate(bits.bitsHi)), dest));
    } else {
        emit(ir::assignConstant(lo, dest));
    }
}

void CodeGeneratingVisitor::emitIncDec(int name, const type::Type& valueType, bool increment) {
    if (type::isFloating(valueType)) {
        const int one = addScratchValue(valueType);
        emitFloatingConstant(one, util::floatingOne(valueType.getSize()));
        if (increment) {
            emit(ir::add(name, one, name));
        } else {
            emit(ir::sub(name, one, name));
        }
        return;
    }
    if (valueType.isPointer()) {
        const int one = addScratchValue(type::signedInteger());
        emit(ir::assignConstant(id("1"), one));
        emitAdditive(increment ? type::ArithmeticOp::Add : type::ArithmeticOp::Sub,
                valueType, type::signedInteger(), name, one, name);
        return;
    }
    if (increment) {
        emit(ir::inc(name, 1));
    } else {
        emit(ir::dec(name, 1));
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    const symbols::ValueEntry* operand = expression.operandSymbol(store_);
    const int operandName = id(*operand);
    emit(ir::assign(operandName, id(*expression.getResultSymbol(store_))));

    emitIncDec(operandName, operand->getType(), expression.op() == type::IncDec::Inc);

    // Dereference (and similar) lvalues: value lives in a temp; store new value through the pointer.
    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), operandName);
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    const int resultSymbolName = id(*expression.getResultSymbol(store_));
    emitIncDec(resultSymbolName, expression.getResultSymbol(store_)->getType(),
            expression.op() == type::IncDec::Inc);

    if (expression.operandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getOperandExpression(), resultSymbolName);
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    if (expression.op() == type::UnaryOp::Sizeof) {
        if (const auto* bytes = expression.sizeofValue(store_)) {
            emit(ir::assignConstant(
                    id(std::to_string(*bytes)),
                    id(*expression.getResultSymbol(store_))));
            return;
        }
        emitSizeofProduct(expression.operandType(),
                id(*expression.getResultSymbol(store_)));
        return;
    }

    expression.visitOperand(*this);

    switch (expression.op()) {
    case type::UnaryOp::Addr:
        // &function designator: SA reuses the designator temp (already emitted FunctionAddress).
        if (expression.getOperandExpression()->holdsFunctionDesignator()) {
            break;
        } else if (auto* lvalue = expression.operandLvalueSymbol(store_)) {
            // &a[i] / &*p: copy the already-computed address into &'s Result.
            emitAssignUnlessSame(id(*lvalue), id(*expression.getResultSymbol(store_)));
        } else {
            emitArrayObjectAddress(*expression.operandSymbol(store_),
                    id(*expression.getResultSymbol(store_)));
        }
        break;
    case type::UnaryOp::Deref: {
        const symbols::ValueEntry* operand = expression.operandSymbol(store_);
        const symbols::ValueEntry* result = expression.getResultSymbol(store_);
        const symbols::ValueEntry* lvalue = expression.getLvalueSymbol(store_);
        if (operand->getType().isPointer()) {
            if (type::isPointerToBareFunction(operand->getType())) {
                emitAssignUnlessSame(id(*operand), id(*result));
                break;
            }
            if (lvalue && id(*result) == id(*lvalue)) {
                emitAssignUnlessSame(id(*operand), id(*result));
            } else {
                emitPointerLoad(*operand, id(*result));
            }
        } else if (expression.operandType().isArray()) {
            emitArrayObjectAddress(*operand, id(*lvalue));
            if (id(*result) != id(*lvalue)) {
                emitPointerLoad(*lvalue, id(*result));
            }
        }
        break;
    }
    case type::UnaryOp::Plus:
        emit(ir::assign(
                convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case type::UnaryOp::Minus:
        emit(ir::unaryMinus(convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case type::UnaryOp::BitNot:
        emit(ir::unaryNot(convertedResult(*expression.getOperandExpression()),
                id(*expression.getResultSymbol(store_))));
        break;
    case type::UnaryOp::LogicalNot:
        emit(ir::zeroCompare(convertedResult(*expression.getOperandExpression())));
        emit(ir::jump(id(*expression.getTruthyLabel(store_)), JumpCondition::IF_EQUAL));
        emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
        emit(ir::jump(id(*expression.getFalsyLabel(store_))));
        emit(ir::label(id(*expression.getTruthyLabel(store_))));
        emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
        emit(ir::label(id(*expression.getFalsyLabel(store_))));
        break;
    case type::UnaryOp::Sizeof:
        break;
    }
}

void CodeGeneratingVisitor::visit(ast::StatementExpression& expression) {
    expression.body().accept(*this);
    // A decayed array value still needs its address emitted.
    auto* last = expression.valueExpression();
    if (last != nullptr && last->hasResultSymbol(store_)) {
        convertedResult(*last);
    }
}

void CodeGeneratingVisitor::visit(ast::GenericSelection& expression) {
    if (!expression.hasSelected()) {
        return;
    }
    expression.selectedExpression().accept(*this);
}

void CodeGeneratingVisitor::visit(ast::CompoundLiteral& expression) {
    expression.initializer().accept(*this);
    auto* object = objectHome(expression);
    if (!object) {
        return;
    }
    const auto& fieldStores = store_.structFieldInits(&expression);
    if (!fieldStores.empty()) {
        emitStructFieldInits(id(*object), fieldStores);
        return;
    }
    if (expression.initializer().hasResultSymbol(store_)) {
        emit(ir::assign(convertedResult(expression.initializer()), id(*object)));
    }
}

void CodeGeneratingVisitor::visit(ast::TypeNameExpression&) {
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    // A cast to void evaluates the operand and discards it.
    if (expression.isVoidValue()) {
        return;
    }
    auto* source = expression.operandSymbol(store_);
    auto* dest = expression.getResultSymbol(store_);
    // Only true array objects need AddressOf. Multi-dim rows already hold a decayed pointer
    // in the result symbol while expression type may still be array.
    if (source->getType().isArray()) {
        emit(ir::addressOf(id(*source), id(*dest)));
    } else {
        emitConvert(id(*source), id(*dest), source->getType(), dest->getType());
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
    const type::ArithmeticOp op = expression.op();
    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*resultSym);

    switch (op) {
    case type::ArithmeticOp::Add:
    case type::ArithmeticOp::Sub:
        emitAdditive(op, leftType, rightType, leftName, rightName, resultName);
        return;
    case type::ArithmeticOp::Mul:
    case type::ArithmeticOp::Div:
    case type::ArithmeticOp::Mod:
        emitMulDiv(op, leftName, rightName, resultName, resultSym->getType());
        return;
    }
}

void CodeGeneratingVisitor::emitAdditive(type::ArithmeticOp op, const type::Type& leftType, const type::Type& rightType,
        int leftName, int rightName, int resultName) {
    const type::PointerArithmeticInfo ptrArith = type::classifyPointerArithmetic(leftType, rightType, op);
    switch (ptrArith.form) {
    case type::PointerArithmeticForm::None:
        break;
    case type::PointerArithmeticForm::PtrPlusInt:
    case type::PointerArithmeticForm::IntPlusPtr:
    case type::PointerArithmeticForm::PtrMinusInt: {
        const bool intLeft = ptrArith.form == type::PointerArithmeticForm::IntPlusPtr;
        const bool subtract = ptrArith.form == type::PointerArithmeticForm::PtrMinusInt;
        const int pointerName = intLeft ? rightName : leftName;
        const int indexName = intLeft ? leftName : rightName;
        const type::Type pointee = (intLeft ? rightType : leftType).dereference();
        const ScaledIndex scaled = scaleIndex(pointee, indexName, ptrArith.strideBytes);
        emit(ir::pointerOffset(pointerName, scaled.name, scaled.strideBytes, resultName, subtract));
        return;
    }
    case type::PointerArithmeticForm::PtrMinusPtr: {
        const type::Type pointee = leftType.dereference();
        if (type::hasComputableRuntimeSize(pointee)) {
            const int size = addScratchValue(type::signedInteger());
            emitSizeofProduct(pointee, size);
            const int bytes = addScratchValue(type::signedInteger());
            emit(ir::pointerDiff(leftName, rightName, 1, bytes));
            emitIntegerMulDiv(type::ArithmeticOp::Div, bytes, size, resultName, type::signedInteger());
            return;
        }
        emit(ir::pointerDiff(leftName, rightName, ptrArith.strideBytes, resultName));
        return;
    }
    case type::PointerArithmeticForm::Invalid:
        throw std::logic_error("pointer arithmetic Invalid should not reach codegen");
    }
    switch (op) {
    case type::ArithmeticOp::Add:
        emit(ir::add(leftName, rightName, resultName));
        return;
    case type::ArithmeticOp::Sub:
        emit(ir::sub(leftName, rightName, resultName));
        return;
    case type::ArithmeticOp::Mul:
    case type::ArithmeticOp::Div:
    case type::ArithmeticOp::Mod:
        throw std::logic_error("emitAdditive: mul/div op");
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*expression.getResultSymbol(store_));
    switch (expression.op()) {
    case type::ShiftOp::Shl:
        emit(ir::shl(leftName, rightName, resultName));
        break;
    case type::ShiftOp::Shr:
        emit(ir::shr(leftName, rightName, resultName,
                type::valueIsSigned(expression.getResultSymbol(store_)->getType())));
        break;
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const auto* leftSym = expression.leftOperandSymbol(store_);
    const auto* rightSym = expression.rightOperandSymbol(store_);
    const type::Type uac = type::usualArithmeticResult(leftSym->getType(), rightSym->getType());
    const bool signedRel = type::valueIsSigned(uac);
    emit(ir::valueCompare(
            convertedResult(*expression.getLeftOperand()),
            convertedResult(*expression.getRightOperand()),
            signedRel));

    const int truthyLabel = id(*expression.getTruthyLabel(store_));
    switch (expression.op()) {
    case type::ComparisonOp::Gt:
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE, signedRel));
        break;
    case type::ComparisonOp::Lt:
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW, signedRel));
        break;
    case type::ComparisonOp::Le:
        emit(ir::jump(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL, signedRel));
        break;
    case type::ComparisonOp::Ge:
        emit(ir::jump(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL, signedRel));
        break;
    case type::ComparisonOp::Eq:
        emit(ir::jump(truthyLabel, JumpCondition::IF_EQUAL));
        break;
    case type::ComparisonOp::Ne:
        emit(ir::jump(truthyLabel, JumpCondition::IF_NOT_EQUAL));
        break;
    }

    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
    emit(ir::jump(id(*expression.getFalsyLabel(store_))));
    emit(ir::label(truthyLabel));
    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
    emit(ir::label(id(*expression.getFalsyLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const int leftName = convertedResult(*expression.getLeftOperand());
    const int rightName = convertedResult(*expression.getRightOperand());
    const int resultName = id(*expression.getResultSymbol(store_));
    switch (expression.op()) {
    case type::BitwiseOp::BitAnd:
        emit(ir::andOp(leftName, rightName, resultName));
        break;
    case type::BitwiseOp::BitOr:
        emit(ir::orOp(leftName, rightName, resultName));
        break;
    case type::BitwiseOp::BitXor:
        emit(ir::xorOp(leftName, rightName, resultName));
        break;
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));
    emit(ir::zeroCompare(convertedResult(*expression.getLeftOperand())));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(convertedResult(*expression.getRightOperand())));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_EQUAL));
    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    emit(ir::assignConstant(id("1"), id(*expression.getResultSymbol(store_))));
    emit(ir::zeroCompare(convertedResult(*expression.getLeftOperand())));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    emit(ir::zeroCompare(convertedResult(*expression.getRightOperand())));
    emit(ir::jump(id(*expression.getExitLabel(store_)), JumpCondition::IF_NOT_EQUAL));
    emit(ir::assignConstant(id("0"), id(*expression.getResultSymbol(store_))));

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::ConditionalExpression& expression) {
    // Void arms produce no value; the arms still run for their side effects.
    const bool valueless = expression.isVoidValue();
    expression.visitCondition(*this);
    emit(ir::zeroCompare(convertedResult(*expression.getCondition())));
    emit(ir::jump(id(*expression.getFalsyLabel(store_)), JumpCondition::IF_EQUAL));

    expression.visitTrueExpression(*this);
    if (!valueless) {
        emit(ir::assign(
                convertedResult(*expression.getTrueExpression()),
                id(*expression.getResultSymbol(store_))));
    }
    emit(ir::jump(id(*expression.getExitLabel(store_))));

    emit(ir::label(id(*expression.getFalsyLabel(store_))));
    expression.visitFalseExpression(*this);
    if (!valueless) {
        emit(ir::assign(
                convertedResult(*expression.getFalseExpression()),
                id(*expression.getResultSymbol(store_))));
    }

    emit(ir::label(id(*expression.getExitLabel(store_))));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    const type::AssignOp op = expression.op();
    const int resultName = id(*expression.getResultSymbol(store_));
    const int rightName = convertedResult(*expression.getRightOperand());
    switch (op) {
    case type::AssignOp::AddAssign:
    case type::AssignOp::SubAssign: {
        const type::Type leftType = expression.getResultSymbol(store_)->getType();
        const type::Type rightType = expression.rightOperandSymbol(store_)->getType();
        emitAdditive(op == type::AssignOp::AddAssign ? type::ArithmeticOp::Add : type::ArithmeticOp::Sub,
                leftType, rightType, resultName, rightName, resultName);
        break;
    }
    case type::AssignOp::MulAssign:
        emitMulDiv(type::ArithmeticOp::Mul, resultName, rightName, resultName,
                expression.getResultSymbol(store_)->getType());
        break;
    case type::AssignOp::DivAssign:
        emitMulDiv(type::ArithmeticOp::Div, resultName, rightName, resultName,
                expression.getResultSymbol(store_)->getType());
        break;
    case type::AssignOp::ModAssign:
        emitMulDiv(type::ArithmeticOp::Mod, resultName, rightName, resultName,
                expression.getResultSymbol(store_)->getType());
        break;
    case type::AssignOp::AndAssign:
        emit(ir::andOp(resultName, rightName, resultName));
        break;
    case type::AssignOp::XorAssign:
        emit(ir::xorOp(resultName, rightName, resultName));
        break;
    case type::AssignOp::OrAssign:
        emit(ir::orOp(resultName, rightName, resultName));
        break;
    case type::AssignOp::ShlAssign:
        emit(ir::shl(resultName, rightName, resultName));
        break;
    case type::AssignOp::ShrAssign:
        emit(ir::shr(resultName, rightName, resultName,
                type::valueIsSigned(expression.getResultSymbol(store_)->getType())));
        break;
    case type::AssignOp::Assign:
        if (expression.leftOperandLvalueSymbol(store_)) {
            emit(ir::assign(rightName, resultName));
            emitLvalueStore(*expression.getLeftOperand(), resultName);
        } else {
            emit(ir::assign(rightName, resultName));
        }
        return;
    }

    if (expression.leftOperandLvalueSymbol(store_)) {
        emitLvalueStore(*expression.getLeftOperand(), resultName);
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
    // The comma aliases the right operand's Result without its Lvalue, so a decayed array has
    // to be materialized here: consumers such as ArrayAccess read Result without converting it.
    if (expression.hasRightOperandSymbol(store_)) {
        convertedResult(*expression.getRightOperand());
    }
}

} // namespace codegen
