#ifndef TYPEDEFREGISTRY_H_
#define TYPEDEFREGISTRY_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Typedef name table for one translation unit (lexer feedback + type lookup).
//
// Layer split:
// - FiniteAutomaton: keyword lookup first, then has(name) -> emit typedef_name.
//   FA does not consult identifier shadows.
// - TokenStream: reclassifies via LexIdContext + isIdentifierShadow.
//
// Write protocol (typedef / shadows / pending): see ParseEnvironment header.
// Object shadows: brace scopes TokenStream push on `{`, pop on `}`. File-scope
// shadows: addIdentifierShadow auto-opens a root frame when the stack is empty.
// pop on empty stack is a no-op so extra `}` does not throw.
// Sessions are single-shot per compile; no reset API (fresh LexicalSession each TU).
class TypedefRegistry {
public:
    void add(const std::string& name, const type::Type& type);
    bool has(const std::string& name) const;
    std::optional<type::Type> tryLookup(const std::string& name) const;

    // Object declarators that reuse a typedef spelling (AST reduce records;
    // TokenStream pushes/pops brace frames). Consulted by reclassify, not FA.
    // Auto-root when empty (file-scope objects); see class comment.
    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(const std::string& name) const;
    void pushIdentifierShadowScope();
    void popIdentifierShadowScope();

    // Parameter names that reuse a typedef spelling: recorded when the param is
    // reduced; flushed into the next brace scope on `{` (any `{`, not only the
    // function body - intermediate braces are a product limit); cleared on `;`
    // so prototypes do not poison later typedef uses.
    void addPendingParameterShadow(const std::string& name);
    void flushPendingParameterShadows();
    void clearPendingParameterShadows();

private:
    std::map<std::string, type::Type> table_;
    std::vector<std::set<std::string>> identifierShadowScopes_;
    std::set<std::string> pendingParameterShadows_;
};

} // namespace scanner

#endif // TYPEDEFREGISTRY_H_
