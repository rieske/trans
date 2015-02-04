#include "DeclarationSpecifiers.h"

#include <stdexcept>

namespace ast {

const std::string DeclarationSpecifiers::ID = "<decl_specs>";

DeclarationSpecifiers::DeclarationSpecifiers() {
}

DeclarationSpecifiers::DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    typeSpecifiers.push_back(typeSpecifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(TypeQualifier typeQualifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    typeQualifiers.push_back(typeQualifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    storageSpecifiers.push_back(storageSpecifier);
}

void DeclarationSpecifiers::accept(AbstractSyntaxTreeVisitor& visitor) {
    throw std::runtime_error { "not implemented: DeclarationSpecifiers::accept" };
    //visitor.visit(*this);
}

} /* namespace ast */
