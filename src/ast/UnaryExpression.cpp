#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

UnaryExpression::UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression)
    : SingleOperandExpression(std::move(castExpression), std::move(unaryOperator)) {
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
    return getOperator()->getKind() == OperatorKind::Deref;
}

bool UnaryExpression::evaluateConstant(type::IntegerConstant& value) const {
    const OperatorKind op = getOperator()->getKind();
    if ((op == OperatorKind::Sizeof || op == OperatorKind::AddressOf) && foldedInteger_) {
        value = type::fromLiteralBits(static_cast<type::Bits>(*foldedInteger_), type::signedInteger());
        return true;
    }
    if (op == OperatorKind::AddressOf) {
        return false;
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

void UnaryExpression::setFoldedInteger(long value) {
    foldedInteger_ = value;
}

bool UnaryExpression::hasFoldedInteger() const {
    return foldedInteger_.has_value();
}

long UnaryExpression::foldedInteger() const {
    return *foldedInteger_;
}

bool UnaryExpression::isSizeof() const {
    return getOperator()->getKind() == OperatorKind::Sizeof;
}

} // namespace ast
