#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

TypeCast::TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression { std::move(castExpression), std::unique_ptr<Operator> { new Operator(typeSpecifier.getName()) } }, typeSpecifier {
                typeSpecifier } {
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> TypeCast::typeAtParseTime(const ParseEnvironment& environment) const {
    TypeSpecifier spec = typeSpecifier;
    if (!spec.resolveTypeofAtParseTime(environment) || !spec.hasType()) {
        return std::nullopt;
    }
    return spec.getType();
}

const TypeSpecifier& TypeCast::getTypeSpecifier() const {
    return typeSpecifier;
}

TypeSpecifier& TypeCast::getTypeSpecifier() {
    return typeSpecifier;
}

bool TypeCast::isLval() const {
    return false;
}

bool TypeCast::evaluateConstant(type::IntegerConstant& value) const {
    if (!_operand || !_operand->evaluateConstant(value)) {
        return false;
    }
    if (!hasExpressionType() && !typeSpecifier.hasType()) {
        return false;
    }
    const type::Type dest = hasExpressionType() ? getType() : typeSpecifier.getType();
    if (!type::isIntegral(dest) && !type::isBoolean(dest) && !dest.isPointer()) {
        return false;
    }
    value = type::convert(value, dest);
    return true;
}

} // namespace ast

