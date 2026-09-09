#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"
#include "Declarator.h"
#include "FormalArgument.h"
#include "InitializedDeclarator.h"
#include "TypeSpecifier.h"
#include "VlaExpressionTable.h"
#include "scanner/LexicalSession.h"
#include "types/IntegerConstant.h"
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

    type::Type ensureStructTag(const std::string& tag);

    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    void defineObject(const std::string& name, type::Type type);
    std::optional<type::Type> lookupObject(const std::string& name) const;
    // Innermost ordinary binding: transients, then object vs enumerator by scope.
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

    bool enumeratorInCurrentScope(const std::string& name) const;
    bool addEnumerator(std::string name);
    bool addEnumerator(std::string name, type::IntegerConstant value);
    bool lookupEnumConstant(const std::string& name, type::IntegerConstant& value) const;
    // Innermost ordinary binding in the session tables. objectType set when an
    // object is nearer or tied; enumerator set when an enumerator is nearer.
    struct OrdinaryBinding {
        std::optional<type::Type> objectType;
        std::optional<type::IntegerConstant> enumerator;
    };
    OrdinaryBinding innermostOrdinary(const std::string& name) const;
    bool lookupInnermostEnumerator(const std::string& name, type::IntegerConstant& value) const;
    // Finishes the innermost open enum body. Non-empty tag is registered.
    struct ClosedEnum {
        type::Type underlying { type::signedInteger() };
        std::vector<Enumerator> enumerators;
    };
    ClosedEnum endEnumDefinition(const std::string& tag = {});
    std::optional<type::Type> lookupEnumTag(const std::string& tag) const;

    VlaExpressionTable& vlaExpressions() { return *vlas_; }
    const VlaExpressionTable& vlaExpressions() const { return *vlas_; }
    std::shared_ptr<VlaExpressionTable> vlaExpressionsShared() const { return vlas_; }

private:
    scanner::LexicalSession& session_;
    const ParseEnvironment* tagParent_ { nullptr };
    bool gnuExtensions_ { true };
    std::shared_ptr<VlaExpressionTable> vlas_;
    std::map<std::string, type::Type> transients_;
    std::map<std::string, type::Type> structTags_;
    std::map<std::string, type::Type> enumTags_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
