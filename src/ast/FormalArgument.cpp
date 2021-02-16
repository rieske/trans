#include "FormalArgument.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string FormalArgument::ID { "<param_decl>" };

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers) :
        specifiers { specifiers }
{
}

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers, std::unique_ptr<Declarator> declarator) :
        specifiers { specifiers },
        declarator { std::move(declarator) }
{
}

ast::FormalArgument::FormalArgument(FormalArgument&& rhs) :
        specifiers { std::move(rhs.specifiers) },
        declarator { std::move(rhs.declarator) }
{
}

void FormalArgument::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FormalArgument::visitSpecifiers(AbstractSyntaxTreeVisitor& visitor) {
    specifiers.accept(visitor);
}

void FormalArgument::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

std::unique_ptr<FundamentalType> FormalArgument::getType() const {
    // FIXME: terribly wrong
    auto baseType = specifiers.getTypeSpecifiers().at(0).getType();
    return declarator->getFundamentalType(*baseType);
}

std::string FormalArgument::getName() const {
    return declarator->getName();
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    return declarator->getContext();
}

} // namespace ast

