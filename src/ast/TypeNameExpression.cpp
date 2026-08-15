#include "TypeNameExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"

namespace ast {

TypeNameExpression::TypeNameExpression(TypeSpecifier typeSpecifier, translation_unit::Context context) :
        typeSpecifier_ { std::move(typeSpecifier) },
        context_ { std::move(context) } {
}

void TypeNameExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> TypeNameExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    TypeSpecifier spec = typeSpecifier_;
    if (!spec.resolveTypeofAtParseTime(environment) || !spec.hasType()) {
        return std::nullopt;
    }
    return spec.getType();
}

translation_unit::Context TypeNameExpression::getContext() const {
    return context_;
}

TypeSpecifier& TypeNameExpression::typeSpecifier() {
    return typeSpecifier_;
}

const TypeSpecifier& TypeNameExpression::typeSpecifier() const {
    return typeSpecifier_;
}

} // namespace ast
