#include "ParameterDeclaration.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"
#include "types/BaseType.h"
#include "types/Type.h"

namespace ast {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TypeSpecifier type, std::unique_ptr<Declaration> declaration) :
        type { type }, declaration { std::move(declaration) } {
}

ParameterDeclaration::~ParameterDeclaration() {
}

void ParameterDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

Type ParameterDeclaration::getTypeInfo() const {
    return {type.getType(), declaration->getDereferenceCount()};
}

}
