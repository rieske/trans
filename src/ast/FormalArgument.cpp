#include "FormalArgument.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"
#include "translation_unit/Context.h"
#include "types/BaseType.h"
#include "types/Type.h"

namespace ast {

const std::string FormalArgument::ID { "<param_decl>" };

FormalArgument::FormalArgument(TypeSpecifier type, std::unique_ptr<Declaration> declaration) :
        type { type },
        declaration { std::move(declaration) }
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

void FormalArgument::visitDeclaration(AbstractSyntaxTreeVisitor& visitor) {
    declaration->accept(visitor);
}

Type FormalArgument::getType() const {
    return {type.getType(), declaration->getDereferenceCount()};
}

std::string FormalArgument::getName() const {
    return declaration->getName();
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    return declaration->getContext();
}

}
