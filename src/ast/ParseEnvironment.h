#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <optional>
#include <string>

#include <memory>

#include "DeclarationSpecifiers.h"
#include "FormalArgument.h"
#include "InitializedDeclarator.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace ast {

class Expression;

// Parse-time tags, typedefs, object types, and enumerators for one TU.
// Caller owns session; it must outlive this environment.
class ParseEnvironment {
public:
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

private:
    void beginEnumDefinition();

    scanner::LexicalSession& session_;
    ParseEnvironment* tagParent_ { nullptr };
    std::map<std::string, type::Type> structTags_;
    std::optional<long> nextEnumeratorValue_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
