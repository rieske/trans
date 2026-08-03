#include "TypeNameExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"

namespace ast {

TypeNameExpression::TypeNameExpression(TypeSpecifier typeSpecifier, translation_unit::Context context) :
        typeName { std::move(typeSpecifier), nullptr },
        context { std::move(context) } {
}

TypeNameExpression::TypeNameExpression(TypeName typeName, translation_unit::Context context) :
        typeName { std::move(typeName) },
        context { std::move(context) } {
}

void TypeNameExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> TypeNameExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    return typeName.tryResolve(environment);
}

translation_unit::Context TypeNameExpression::getContext() const {
    return context;
}

TypeSpecifier& TypeNameExpression::typeSpecifier() {
    return typeName.spec;
}

const TypeSpecifier& TypeNameExpression::typeSpecifier() const {
    return typeName.spec;
}

TypeName& TypeNameExpression::getTypeName() {
    return typeName;
}

const TypeName& TypeNameExpression::getTypeName() const {
    return typeName;
}

} // namespace ast
