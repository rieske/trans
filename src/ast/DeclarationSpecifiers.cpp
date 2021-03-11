#include "DeclarationSpecifiers.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

DeclarationSpecifiers::DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers declarationSpecifiers) :
        DeclarationSpecifiers(declarationSpecifiers)
{
    typeSpecifiers.push_back(typeSpecifier);
}

DeclarationSpecifiers::DeclarationSpecifiers(type::Qualifier typeQualifier, DeclarationSpecifiers declarationSpecifiers) :
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

const std::vector<TypeSpecifier>& DeclarationSpecifiers::getTypeSpecifiers() const {
    return typeSpecifiers;
}

const std::vector<type::Qualifier>& DeclarationSpecifiers::getTypeQualifiers() const {
    return typeQualifiers;
}

const std::vector<StorageSpecifier>& DeclarationSpecifiers::getStorageSpecifiers() const {
    return storageSpecifiers;
}

} // namespace ast
