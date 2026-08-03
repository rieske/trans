#include "CompoundLiteralExpression.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

CompoundLiteralExpression::CompoundLiteralExpression(TypeSpecifier typeSpecifier,
        std::unique_ptr<InitializerListExpression> initializer, translation_unit::Context context)
    : typeSpecifier{std::move(typeSpecifier)},
      initializer{std::move(initializer)},
      context{std::move(context)} {
    lval = true;
}

void CompoundLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
translation_unit::Context CompoundLiteralExpression::getContext() const { return context; }
const TypeSpecifier& CompoundLiteralExpression::getTypeSpecifier() const { return typeSpecifier; }
TypeSpecifier& CompoundLiteralExpression::getTypeSpecifier() { return typeSpecifier; }
InitializerListExpression* CompoundLiteralExpression::getInitializer() const { return initializer.get(); }
bool CompoundLiteralExpression::isLval() const { return true; }




symbols::ValueEntry* CompoundLiteralExpression::lvalueAnnotation(symbols::AnnotationStore& store) const {
    auto* objectSymbol = store.value(this, symbols::ValueSlot::Object);
    if (objectSymbol && objectSymbol->getType().isArray()) {
        return nullptr;
    }
    return objectSymbol;
}

} // namespace ast

