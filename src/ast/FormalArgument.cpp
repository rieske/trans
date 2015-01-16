#include "FormalArgument.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declarator.h"
#include "translation_unit/Context.h"
#include "types/BaseType.h"
#include "types/Type.h"

namespace ast {

const std::string FormalArgument::ID { "<param_decl>" };

FormalArgument::FormalArgument(TypeSpecifier type, std::unique_ptr<Declarator> declarator) :
        type { type },
        declarator { std::move(declarator) }
{
}

FormalArgument::~FormalArgument() {
}

void FormalArgument::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FormalArgument::visitTypeSpecifier(AbstractSyntaxTreeVisitor& visitor) {
    type.accept(visitor);
}

void FormalArgument::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    declarator->accept(visitor);
}

Type FormalArgument::getType() const {
    return {type.getType(), declarator->getDereferenceCount()};
}

std::string FormalArgument::getName() const {
    return declarator->getName();
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    return declarator->getContext();
}

}
