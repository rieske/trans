#include "CompoundLiteralExpression.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

CompoundLiteralExpression::CompoundLiteralExpression(TypeName typeName,
        std::unique_ptr<InitializerListExpression> initializer, translation_unit::Context context)
    : typeName{std::move(typeName)},
      initializer{std::move(initializer)},
      context{std::move(context)} {
    lval = true;
}

void CompoundLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
translation_unit::Context CompoundLiteralExpression::getContext() const { return context; }

std::optional<type::Type> CompoundLiteralExpression::typeAtParseTime(
        const ParseEnvironment& environment) const {
    return typeName.tryResolve(environment);
}
const TypeName& CompoundLiteralExpression::getTypeName() const { return typeName; }
TypeName& CompoundLiteralExpression::getTypeName() { return typeName; }
InitializerListExpression* CompoundLiteralExpression::getInitializer() const { return initializer.get(); }

void CompoundLiteralExpression::setInitializer(std::unique_ptr<InitializerListExpression> init) {
    initializer = std::move(init);
}
bool CompoundLiteralExpression::isLval() const { return true; }

} // namespace ast

