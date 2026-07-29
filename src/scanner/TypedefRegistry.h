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
// - TokenStream (Phase 2+): reclassifies via LexIdContext + isIdentifierShadow.
//
// Object shadows (e.g. `cmp_type cmp_type = ...`) are recorded so later uses
// reclassify as "id". Brace scopes: TokenStream push on `{`, pop on `}`.
// File-scope shadows: addIdentifierShadow auto-opens a root frame when the
// stack is empty (no prior push). pop on empty stack is a no-op so extra `}`
// does not throw; mismatched pops should be caught by TokenStream tests.
// Sessions are single-shot per compile; no reset API (fresh LexicalSession each TU).
class TypedefRegistry {
public:
    void add(const std::string& name, const type::Type& type);
    bool has(const std::string& name) const;
    std::optional<type::Type> tryLookup(const std::string& name) const;

    // Shadows are consulted by TokenStream reclassify, not by the FA.
    // Auto-root when empty (file-scope objects); see class comment.
    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(const std::string& name) const;
    void pushIdentifierShadowScope();
    void popIdentifierShadowScope();

private:
    std::map<std::string, type::Type> table_;
    std::vector<std::set<std::string>> identifierShadowScopes_;
};

} // namespace scanner

#endif // TYPEDEFREGISTRY_H_
