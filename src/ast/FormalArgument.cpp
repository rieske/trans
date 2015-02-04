#include "FormalArgument.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declarator.h"
#include "translation_unit/Context.h"
#include "types/BaseType.h"
#include "types/Type.h"

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

Type FormalArgument::getType() const {
    //return {type.getType(), declarator->getDereferenceCount()};
    throw std::runtime_error { "formal argument get type is not implemented yet" };
}

std::string FormalArgument::getName() const {
    return declarator->getName();
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    return declarator->getContext();
}

}
