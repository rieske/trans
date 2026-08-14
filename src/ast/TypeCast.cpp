#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"

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

bool TypeCast::evaluateConstant(long& value) const {
    // Integer cast of a constant: fold through the operand (width truncation later if needed).
    return _operand && _operand->evaluateConstant(value);
}

} // namespace ast

