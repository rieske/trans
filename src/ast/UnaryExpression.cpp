#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

UnaryExpression::UnaryExpression(type::UnaryOp op, std::unique_ptr<Expression> castExpression) :
        UnaryOpExpression(std::move(castExpression), op)
{
}

void UnaryExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> UnaryExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    auto inner = _operand->typeAtParseTime(environment);
    if (!inner) {
        return std::nullopt;
    }
    switch (op()) {
    case type::UnaryOp::Deref:
        return type::afterLvalueConversion(*inner).indexElement();
    case type::UnaryOp::Addr:
        return type::pointer(*inner);
    case type::UnaryOp::Sizeof:
        return type::signedInteger();
    case type::UnaryOp::LogicalNot: {
        const type::Type converted = type::afterLvalueConversion(*inner);
        if (!type::isProductScalar(converted) && !inner->isFunction()) {
            return std::nullopt;
        }
        return type::signedInteger();
    }
    case type::UnaryOp::BitNot: {
        const type::Type converted = type::afterLvalueConversion(*inner);
        if (!type::isIntegral(converted)) {
            return std::nullopt;
        }
        return type::integerPromote(converted);
    }
    case type::UnaryOp::Plus:
    case type::UnaryOp::Minus: {
        const type::Type converted = type::afterLvalueConversion(*inner);
        if (!type::isArithmeticType(converted)) {
            return std::nullopt;
        }
        return type::integerPromote(converted);
    }
    }
    return std::nullopt;
}

void UnaryExpression::setSizeofValue(symbols::AnnotationStore& store, int bytes) {
    store.setSizeofValue(this, bytes);
}

const int* UnaryExpression::sizeofValue(const symbols::AnnotationStore& store) const {
    return store.sizeofValue(this);
}

bool UnaryExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (op() == type::UnaryOp::Sizeof) {
        if (!_operand || !_operand->hasExpressionType()) {
            return false;
        }
        const auto bytes = type::sizeofObject(operandType(), true);
        if (!bytes) {
            return false;
        }
        value = type::fromLiteralBits(static_cast<type::Bits>(*bytes), type::signedInteger());
        return true;
    }
    type::IntegerConstant operand;
    if (!_operand->evaluateConstant(operand)) {
        return false;
    }
    auto folded = type::foldUnary(op(), operand);
    if (!folded) {
        return false;
    }
    value = *folded;
    return true;
}

void UnaryExpression::setTruthyLabel(symbols::AnnotationStore& store, symbols::LabelEntry truthyLabel) {
    store.setLabel(this, symbols::LabelSlot::Truthy, std::move(truthyLabel));
}

symbols::LabelEntry* UnaryExpression::getTruthyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Truthy);
}

void UnaryExpression::setFalsyLabel(symbols::AnnotationStore& store, symbols::LabelEntry falsyLabel) {
    store.setLabel(this, symbols::LabelSlot::Falsy, std::move(falsyLabel));
}

symbols::LabelEntry* UnaryExpression::getFalsyLabel(symbols::AnnotationStore& store) const {
    return store.label(this, symbols::LabelSlot::Falsy);
}

} // namespace ast

