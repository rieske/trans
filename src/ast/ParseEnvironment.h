#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "InitializedDeclarator.h"
#include "PendingArrayMemberStore.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace ast {

class Expression;

// Parse-time symbol environment for one translation unit: struct/union tags,
// typedef/enum names (via LexicalSession), object types for typeof, and
// deferred ARRAY_SIZE member bounds.
//
// Enumerators: sole authority is session.enums (TU-flat). SA imports via
// AbstractSyntaxTree::parseEnumConstants snapshot at build().
class ParseEnvironment {
public:
    // Caller owns session; it must outlive this environment.
    explicit ParseEnvironment(scanner::LexicalSession& session);
    ParseEnvironment(scanner::LexicalSession& session, ParseEnvironment& parent);

    scanner::LexicalSession& session() { return session_; }

    type::Type ensureStructTag(const std::string& tag);

    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    void defineObject(const std::string& name, type::Type type);
    std::optional<type::Type> lookupObject(const std::string& name) const;
    void maybeDefineParameter(const FormalArgument& argument);
    std::optional<type::Type> typeOf(const Expression& expression) const;

    void registerInitializedDeclaration(
            const DeclarationSpecifiers& specs,
            const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators);
    void maybeRegisterParameterShadow(const std::string& name);

    void addEnumerator(std::string name, std::optional<long> explicitValue = std::nullopt);
    bool lookupEnumConstant(const std::string& name, long& value) const;
    void endEnumDefinition();
    std::map<std::string, long> enumConstantsSnapshot() const;

    PendingArrayMemberStore& pendingArrayMembers() { return pendingArrayMembers_; }
    const PendingArrayMemberStore& pendingArrayMembers() const { return pendingArrayMembers_; }
    PendingArrayMemberStore takePendingArrayMembers() { return std::move(pendingArrayMembers_); }

private:
    void beginEnumDefinition();

    scanner::LexicalSession& session_;
    ParseEnvironment* tagParent_ { nullptr };
    std::map<std::string, type::Type> structTags_;
    std::vector<long> enumNextValueStack_;
    PendingArrayMemberStore pendingArrayMembers_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
