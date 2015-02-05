#ifndef DECLARATIONSPECIFIER_H_
#define DECLARATIONSPECIFIER_H_

#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "StorageSpecifier.h"
#include "TypeQualifier.h"
#include "TypeSpecifier.h"

namespace ast {

class DeclarationSpecifiers: public AbstractSyntaxTreeNode {
public:
    DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers declarationSpecifiers = { });
    DeclarationSpecifiers(TypeQualifier typeQualifier, DeclarationSpecifiers declarationSpecifiers = { });
    DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers declarationSpecifiers = { });
    virtual ~DeclarationSpecifiers() = default;
    DeclarationSpecifiers(const DeclarationSpecifiers&) = default;
    DeclarationSpecifiers(DeclarationSpecifiers&&) = default;

    DeclarationSpecifiers& operator=(const DeclarationSpecifiers&) = default;
    DeclarationSpecifiers& operator=(DeclarationSpecifiers&&) = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    const std::vector<TypeSpecifier>& getTypeSpecifiers() const;
    const std::vector<TypeQualifier>& getTypeQualifiers() const;
    const std::vector<StorageSpecifier>& getStorageSpecifiers() const;

private:
    DeclarationSpecifiers() = default;

    std::vector<TypeSpecifier> typeSpecifiers;
    std::vector<TypeQualifier> typeQualifiers;
    std::vector<StorageSpecifier> storageSpecifiers;
};

} /* namespace ast */

#endif /* DECLARATIONSPECIFIER_H_ */
