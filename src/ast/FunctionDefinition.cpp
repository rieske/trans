#include "FunctionDefinition.h"
#include <stdexcept>
#include "AbstractSyntaxTreeVisitor.h"
namespace ast {
FunctionDefinition::FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator,
        std::unique_ptr<AbstractSyntaxTreeNode> body)
    : returnType{returnType}, declarator{std::move(declarator)}, body{std::move(body)} {}
void FunctionDefinition::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void FunctionDefinition::visitReturnType(AbstractSyntaxTreeVisitor& visitor) { returnType.accept(visitor); }
void FunctionDefinition::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) { declarator->accept(visitor); }
void FunctionDefinition::visitBody(AbstractSyntaxTreeVisitor& visitor) { body->accept(visitor); }
void FunctionDefinition::visitBodyChildren(AbstractSyntaxTreeVisitor& visitor) { body->visitChildren(visitor); }
std::string FunctionDefinition::getName() const { return declarator->getName(); }
std::vector<std::string> FunctionDefinition::parameterNames() const {
    return declarator->formalParameterNames();
}
const DeclarationSpecifiers& FunctionDefinition::getReturnTypeSpecifiers() const { return returnType; }
type::Type FunctionDefinition::getDeclaratorType(const type::Type& baseType) const {
    return declarator->getFundamentalType(baseType);
}
translation_unit::Context FunctionDefinition::getDeclaratorContext() const { return declarator->getContext(); }
} // namespace ast
