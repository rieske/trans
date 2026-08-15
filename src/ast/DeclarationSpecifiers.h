#ifndef DECLARATIONSPECIFIER_H_
#define DECLARATIONSPECIFIER_H_

#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "StorageSpecifier.h"
#include "TypeSpecifier.h"

namespace ast {

class ParseEnvironment;

class DeclarationSpecifiers: public AbstractSyntaxTreeNode {
public:
    DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers declarationSpecifiers = { });
    DeclarationSpecifiers(type::Qualifier typeQualifier, DeclarationSpecifiers declarationSpecifiers = { });
    DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers declarationSpecifiers = { });
    static DeclarationSpecifiers none();

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void resolveTypeof(AbstractSyntaxTreeVisitor& visitor);
    bool resolveTypeofAtParseTime(const ParseEnvironment& environment);
    bool needsSemanticResolve() const;

    const std::vector<TypeSpecifier>& getTypeSpecifiers() const;
    const std::vector<StorageSpecifier>& getStorageSpecifiers() const;
    bool hasStorage(Storage storage) const;
    bool isTypedef() const { return hasStorage(Storage::TYPEDEF); }
    // Combine multi-word type specs (unsigned int, long unsigned, ...) into one Type.
    type::Type getResolvedType() const;
    // Untagged complete struct/union: C11 anonymous member. Uses the stored
    // TypeSpecifier name, not a reconstructed spelling.
    bool isUntaggedCompleteRecord() const;
    // type_name form: identity when a single unqualified spec, otherwise the
    // resolved type with that spec's name (empty when several specs combine).
    TypeSpecifier toTypeSpecifier() const;

private:
    DeclarationSpecifiers() = default;

    std::vector<TypeSpecifier> typeSpecifiers;
    std::vector<type::Qualifier> typeQualifiers;
    std::vector<StorageSpecifier> storageSpecifiers;
};

} // namespace ast

#endif // DECLARATIONSPECIFIER_H_
