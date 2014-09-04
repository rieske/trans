#include "ParameterDeclaration.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"
#include "Declaration.h"
#include "TypeInfo.h"

namespace ast {

const std::string ParameterDeclaration::ID { "<param_decl>" };

ParameterDeclaration::ParameterDeclaration(TypeSpecifier type, std::unique_ptr<Declaration> declaration) :
        type { type },
        declaration { std::move(declaration) } {
    auto basicType = type.getType();
    auto extended_type = this->declaration->getType();
    auto name = this->declaration->getName();
}

ParameterDeclaration::~ParameterDeclaration() {
}

void ParameterDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

TypeInfo ParameterDeclaration::getTypeInfo() const {
    return {type.getType(), declaration->getType()};
}

}
