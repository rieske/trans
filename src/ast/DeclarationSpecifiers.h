#ifndef DECLARATIONSPECIFIER_H_
#define DECLARATIONSPECIFIER_H_

#include <optional>
#include <vector>

#include "AbstractSyntaxTreeNode.h"
#include "FunctionSpecifier.h"
#include "StorageSpecifier.h"
#include "TypeSpecifier.h"

namespace ast {

class ParseEnvironment;

class DeclarationSpecifiers: public AbstractSyntaxTreeNode {
public:
    DeclarationSpecifiers(TypeSpecifier typeSpecifier, DeclarationSpecifiers rest = { });
    DeclarationSpecifiers(type::Qualifier typeQualifier, DeclarationSpecifiers rest = { });
    DeclarationSpecifiers(StorageSpecifier storageSpecifier, DeclarationSpecifiers rest = { });
    DeclarationSpecifiers(FunctionSpecifier functionSpecifier, DeclarationSpecifiers rest = { });
    static DeclarationSpecifiers none();

    void add(TypeSpecifier typeSpecifier);
    void add(type::Qualifier typeQualifier);
    void add(StorageSpecifier storageSpecifier);
    void add(FunctionSpecifier functionSpecifier);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void resolveTypeof(AbstractSyntaxTreeVisitor& visitor);
    bool resolveTypeofAtParseTime(const ParseEnvironment& environment);
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const;
    bool needsSemanticResolve() const;

    std::vector<TypeSpecifier>& getTypeSpecifiers();
    const std::vector<TypeSpecifier>& getTypeSpecifiers() const;
    const std::vector<StorageSpecifier>& getStorageSpecifiers() const;
    bool hasStorage(Storage storage) const;
    const std::vector<FunctionSpecifier>& getFunctionSpecifiers() const;
    bool hasFunctionSpec(FunctionSpec spec) const;
    bool isTypedef() const { return hasStorage(Storage::TYPEDEF); }
    // Combine multi-word type specs (unsigned int, long unsigned, ...) into one Type.
    type::Type getResolvedType() const;
    // Untagged record body (complete or tentative): C11 anonymous member.
    // Uses the stored TypeSpecifier name, not a reconstructed spelling.
    bool isUntaggedRecordBody() const;
    // type_name form: identity when a single unqualified spec, otherwise the
    // resolved type with that spec's name (empty when several specs combine).
    TypeSpecifier toTypeSpecifier() const;

private:
    DeclarationSpecifiers() = default;

    std::vector<TypeSpecifier> typeSpecifiers;
    std::vector<type::Qualifier> typeQualifiers;
    std::vector<StorageSpecifier> storageSpecifiers;
    std::vector<FunctionSpecifier> functionSpecifiers;
};

} // namespace ast

#endif // DECLARATIONSPECIFIER_H_
