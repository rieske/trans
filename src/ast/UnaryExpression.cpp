#include "UnaryExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

UnaryExpression::UnaryExpression(std::string lexeme, std::unique_ptr<Expression> castExpression) :
        UnaryOpExpression(std::move(castExpression), std::move(lexeme))
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
    const std::string op = lexeme();
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
    return lexeme() == "*";
}

void UnaryExpression::setSizeofValue(symbols::AnnotationStore& store, int bytes) {
    store.setSizeofValue(this, bytes);
}

const int* UnaryExpression::sizeofValue(const symbols::AnnotationStore& store) const {
    return store.sizeofValue(this);
}

bool UnaryExpression::evaluateConstant(type::IntegerConstant& value) const {
    if (lexeme() == "sizeof") {
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
    auto folded = type::foldUnary(lexeme(), operand);
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

