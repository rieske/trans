#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

const std::string DeclarationSpecifiers::ID = "<decl_specs>";

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
    visitor.visit(*this);
}

const std::vector<TypeSpecifier>& DeclarationSpecifiers::getTypeSpecifiers() {
    return typeSpecifiers;
}

const std::vector<TypeQualifier>& DeclarationSpecifiers::getTypeQualifiers() {
    return typeQualifiers;
}

const std::vector<StorageSpecifier>& DeclarationSpecifiers::getStorageSpecifiers() {
    return storageSpecifiers;
}

} /* namespace ast */
