#ifndef AST_PARSEENVIRONMENT_H_
#define AST_PARSEENVIRONMENT_H_

#include <map>
#include <optional>
#include <string>

#include <memory>

#include "DeclarationSpecifiers.h"
#include "InitializedDeclarator.h"
#include "scanner/LexicalSession.h"
#include "types/Type.h"

namespace ast {

// Parse-time symbol environment for one translation unit: struct/union tags
// and typedef/enum names (via LexicalSession). Separate from the bottom-up
// reduction stacks on AbstractSyntaxTreeBuilderContext.
//
// Lexer-feedback protocol (production declaration events → PE → registry):
// | Event                         | Who          | API                          |
// | typedef decl reduced          | PE (CSNB)    | registerInitializedDecl...   |
// | object reuses typedef spelling| PE           | (same → registry shadow)     |
// | param reuses typedef spelling | PE / CSNB    | maybeRegisterParameterShadow |
// | `{`                           | TokenStream  | push + flushPending          |
// | `}`                           | TokenStream  | pop                          |
// | `;`                           | TokenStream  | clearPending                 |
// defineTypedef is the registry primitive under those events (also used in tests).
class ParseEnvironment {
public:
    // Caller owns session; it must outlive this environment.
    explicit ParseEnvironment(scanner::LexicalSession& session);

    // Returns existing tag type or creates an incomplete record placeholder for the tag
    // (completed later as struct or union). Sole public tag API.
    type::Type ensureStructTag(const std::string& tag);

    // Registry write primitive (name → Type). Prefer registerInitializedDeclaration
    // for declaration-reduce events; call this only for direct/test registration.
    void defineTypedef(const std::string& name, type::Type type);
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

    // Declaration-event API: typedef alias registration or object-name shadow.
    void registerInitializedDeclaration(
            const DeclarationSpecifiers& specs,
            const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators);

    // Parameter that reuses a typedef spelling: pending until the next `{`
    // (TokenStream flushes into that brace scope). Not body-only: an intermediate
    // `{` mid-parameter-list can flush early (product limit). Prototypes clear on `;`.
    // Nested function-type params with typedef spellings as names are not supported.
    void maybeRegisterParameterShadow(const std::string& name);

    void addEnumerator(std::string name, std::optional<long> explicitValue = std::nullopt);
    bool lookupEnumConstant(const std::string& name, long& value) const;
    // Ends the current enum definition (auto-increment state). Values already live on session.
    void endEnumDefinition();
    // Snapshot of all parse-time enumerators registered on the session (for SA import).
    // Product limit: TU-flat (not C block scope); SA keeps the same map.
    std::map<std::string, long> enumConstantsSnapshot() const;

private:
    void beginEnumDefinition();

    scanner::LexicalSession& session_;
    std::map<std::string, type::Type> structTags_;
    // Next auto-value while an enum is open (set on first addEnumerator).
    // Nested enum-in-const_exp unsupported. Values live only on session_.enums.
    std::optional<long> nextEnumeratorValue_;
};

} // namespace ast

#endif // AST_PARSEENVIRONMENT_H_
