#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <optional>
#include <string>

#include <memory>

#include "DeclarationSpecifiers.h"
#include "Declarator.h"
#include "FormalArgument.h"
#include "InitializedDeclarator.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace ast {

class Block;
class Expression;

// Parse-time tags, typedefs, object types, and enumerators for one TU.
// Caller owns session; it must outlive this environment.
class ParseEnvironment {
public:
    explicit ParseEnvironment(scanner::LexicalSession& session);
    ParseEnvironment(scanner::LexicalSession& session, const ParseEnvironment& parent);
    // Shares the session and parent lookup chain. Does not copy tags or transients.
    static ParseEnvironment nestedIn(const ParseEnvironment& enclosing);

    scanner::LexicalSession& session() { return session_; }
    const scanner::LexicalSession& session() const { return session_; }

    type::Type ensureStructTag(const std::string& tag);

    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    void defineObject(const std::string& name, type::Type type);
    std::optional<type::Type> lookupObject(const std::string& name) const;
    // Objects, statement-expression locals, then enumerators.
    std::optional<type::Type> lookupValueType(const std::string& name) const;
    void defineTransient(const std::string& name, type::Type type);
    void bindBlockDeclarations(const Block& block);
    void maybeDefineParameter(const FormalArgument& argument);
    std::optional<type::Type> typeOf(const Expression& expression) const;

    void setGnuExtensions(bool enabled) { gnuExtensions_ = enabled; }
    bool gnuExtensions() const { return gnuExtensions_; }

    void registerInitializedDeclaration(
            const DeclarationSpecifiers& specs,
            const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators);
    void tryDefineObject(const DeclarationSpecifiers& specs, Declarator& declarator);
    void maybeRegisterParameterShadow(const std::string& name);

    void addEnumerator(std::string name, std::optional<long> explicitValue = std::nullopt);
    bool lookupEnumConstant(const std::string& name, long& value) const;
    // Finishes the open enum body; returns the underlying type. Non-empty tag is registered.
    type::Type endEnumDefinition(const std::string& tag = {});
    std::optional<type::Type> lookupEnumTag(const std::string& tag) const;
    std::map<std::string, long> enumConstantsSnapshot() const;

private:
    struct EnumBody {
        long next { 0 };
        long min { 0 };
        long max { 0 };
    };

    scanner::LexicalSession& session_;
    const ParseEnvironment* tagParent_ { nullptr };
    bool gnuExtensions_ { true };
    std::map<std::string, type::Type> transients_;
    std::map<std::string, type::Type> structTags_;
    std::map<std::string, type::Type> enumTags_;
    std::optional<EnumBody> enumBody_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
