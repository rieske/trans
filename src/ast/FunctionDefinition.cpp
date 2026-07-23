#include "FunctionDefinition.h"
#include <stdexcept>
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
FunctionDefinition::FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator,
        std::unique_ptr<AbstractSyntaxTreeNode> body)
    : returnType{returnType}, declarator{std::move(declarator)}, body{std::move(body)} {}
void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void FunctionDefinition::visitReturnType(AbstractSyntaxTreeVisitor& visitor) { returnType.accept(visitor); }
void FunctionDefinition::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) { declarator->accept(visitor); }
void FunctionDefinition::visitBody(AbstractSyntaxTreeVisitor& visitor) { body->accept(visitor); }
void FunctionDefinition::visitBodyChildren(AbstractSyntaxTreeVisitor& visitor) { body->visitChildren(visitor); }
void FunctionDefinition::setSymbol(symbols::FunctionEntry symbol) {
    symbols::AnnotationStore::current().functionFrame(this).symbol =
            std::make_unique<symbols::FunctionEntry>(std::move(symbol));
}
bool FunctionDefinition::hasSymbol() const {
    auto* f = symbols::AnnotationStore::current().functionFrameIfAny(this);
    return f && f->symbol;
}
symbols::FunctionEntry* FunctionDefinition::getSymbol() const {
    auto* f = symbols::AnnotationStore::current().functionFrameIfAny(this);
    if (!f || !f->symbol) throw std::runtime_error{"FunctionDefinition::getSymbol() == nullptr"};
    return f->symbol.get();
}
void FunctionDefinition::setLocalVariables(std::map<std::string, symbols::ValueEntry> localVariables) {
    symbols::AnnotationStore::current().functionFrame(this).locals = std::move(localVariables);
}
void FunctionDefinition::setArguments(std::vector<symbols::ValueEntry> arguments) {
    symbols::AnnotationStore::current().functionFrame(this).arguments = std::move(arguments);
}
void FunctionDefinition::setParameterNames(std::vector<std::string> names) {
    symbols::AnnotationStore::current().functionFrame(this).parameterNames = std::move(names);
}
const std::vector<std::string>& FunctionDefinition::getParameterNames() const {
    return symbols::AnnotationStore::current().functionFrame(const_cast<FunctionDefinition*>(this)).parameterNames;
}
std::map<std::string, symbols::ValueEntry> FunctionDefinition::getLocalVariables() const {
    auto* f = symbols::AnnotationStore::current().functionFrameIfAny(this);
    return f ? f->locals : std::map<std::string, symbols::ValueEntry>{};
}
std::vector<symbols::ValueEntry> FunctionDefinition::getArguments() const {
    auto* f = symbols::AnnotationStore::current().functionFrameIfAny(this);
    return f ? f->arguments : std::vector<symbols::ValueEntry>{};
}
std::string FunctionDefinition::getName() const { return declarator->getName(); }
const DeclarationSpecifiers& FunctionDefinition::getReturnTypeSpecifiers() const { return returnType; }
type::Type FunctionDefinition::getDeclaratorType(const type::Type& baseType) const {
    return declarator->getFundamentalType(baseType);
}
translation_unit::Context FunctionDefinition::getDeclaratorContext() const { return declarator->getContext(); }
} // namespace ast
