#include "InitializedDeclarator.h"
#include <stdexcept>
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
InitializedDeclarator::InitializedDeclarator(std::unique_ptr<Declarator> declarator, std::unique_ptr<Expression> initializer)
    : declarator{std::move(declarator)}, initializer{std::move(initializer)} {}
void InitializedDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void InitializedDeclarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
    if (initializer) initializer->accept(visitor);
}
std::string InitializedDeclarator::getName() const { return declarator->getName(); }
Declarator* InitializedDeclarator::getDeclarator() const { return declarator.get(); }
bool InitializedDeclarator::hasInitializer() const { return !!initializer; }
Expression* InitializedDeclarator::getInitializer() const { return initializer.get(); }
symbols::ValueEntry* InitializedDeclarator::getInitializerHolder() const {
    return initializer->getResultSymbol();
}
translation_unit::Context InitializedDeclarator::getContext() const { return declarator->getContext(); }
void InitializedDeclarator::setHolder(symbols::ValueEntry holder) {
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::Holder, std::move(holder));
}
symbols::ValueEntry* InitializedDeclarator::getHolder() const {
    auto* h = symbols::AnnotationStore::current().value(this, symbols::ValueSlot::Holder);
    if (!h) throw std::runtime_error{"InitializedDeclarator::getHolder() == nullptr"};
    return h;
}
void InitializedDeclarator::addStructFieldInit(StructFieldInit init) { structFieldInits.push_back(std::move(init)); }
const std::vector<StructFieldInit>& InitializedDeclarator::getStructFieldInits() const { return structFieldInits; }
type::Type InitializedDeclarator::getFundamentalType(const type::Type& baseType) {
    return declarator->getFundamentalType(baseType);
}
} // namespace ast
