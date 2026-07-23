#include "CompoundLiteralExpression.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
CompoundLiteralExpression::CompoundLiteralExpression(TypeSpecifier typeSpecifier,
        std::unique_ptr<InitializerListExpression> initializer, translation_unit::Context context)
    : typeSpecifier{std::move(typeSpecifier)}, initializer{std::move(initializer)}, context{std::move(context)} { lval = true; }
void CompoundLiteralExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
translation_unit::Context CompoundLiteralExpression::getContext() const { return context; }
const TypeSpecifier& CompoundLiteralExpression::getTypeSpecifier() const { return typeSpecifier; }
InitializerListExpression* CompoundLiteralExpression::getInitializer() const { return initializer.get(); }
bool CompoundLiteralExpression::isLval() const { return true; }
symbols::ValueEntry* CompoundLiteralExpression::getObjectSymbol() const {
    return symbols::AnnotationStore::current().value(this, symbols::ValueSlot::Object);
}
void CompoundLiteralExpression::setObjectSymbol(symbols::ValueEntry symbol) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::Object, std::move(symbol));
}
symbols::ValueEntry* CompoundLiteralExpression::getLvalueSymbol() const {
    auto* objectSymbol = getObjectSymbol();
    if (objectSymbol && objectSymbol->getType().isArray()) return nullptr;
    return objectSymbol;
}
void CompoundLiteralExpression::addStructFieldInit(StructFieldInit init) { structFieldInits.push_back(std::move(init)); }
const std::vector<StructFieldInit>& CompoundLiteralExpression::getStructFieldInits() const { return structFieldInits; }
} // namespace ast
