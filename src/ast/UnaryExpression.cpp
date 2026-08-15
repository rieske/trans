#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression(std::move(castExpression), std::move(unaryOperator))
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
    const std::string op = getOperator()->getLexeme();
    if (op == "*") {
        return type::afterLvalueConversion(*inner).indexElement();
    }
    if (op == "&") {
        return type::pointer(*inner);
    }
    if (op == "sizeof") {
        return type::signedInteger();
    }
    const type::Type converted = type::afterLvalueConversion(*inner);
    if (op == "!") {
        if (!type::isProductScalar(converted) && !type::isBareFunction(*inner)) {
            return std::nullopt;
        }
        return type::signedInteger();
    }
    if (op == "~") {
        if (!type::isIntegral(converted)) {
            return std::nullopt;
        }
        return type::integerPromote(converted);
    }
    if (op == "+" || op == "-") {
        if (!type::isArithmeticType(converted)) {
            return std::nullopt;
        }
        return type::integerPromote(converted);
    }
    return std::nullopt;
}

bool UnaryExpression::isLval() const {
    // Only dereference yields an lvalue; +a, -a, !a, &a are rvalues.
    return getOperator()->getLexeme() == "*";
}

void UnaryExpression::setSizeofValue(int bytes) {
    sizeofValue = bytes;
}

int UnaryExpression::getSizeofValue() const {
    return sizeofValue;
}

bool UnaryExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (getOperator()->getLexeme() == "sizeof" && sizeofValue >= 0) {
        value = type::fromLiteralBits(static_cast<type::Bits>(sizeofValue), type::signedInteger());
        return true;
    }
    type::IntegerConstant operand;
    if (!_operand->evaluateConstant(operand)) {
        return false;
    }
    auto folded = type::foldUnary(getOperator()->getLexeme(), operand);
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

