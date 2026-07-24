#ifndef TYPEDEFREGISTRY_H_
#define TYPEDEFREGISTRY_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Process-wide typedef name table used by the lexer hack: identifiers that
// name typedef'd types must be returned as token type "typedef_name".
//
// When a declarator reuses a typedef name as an ordinary object (e.g.
// `cmp_type cmp_type = ...`), that name is recorded as an identifier shadow
// so later uses in the same brace scope lex as "id" rather than "typedef_name".
// Shadow scopes track `{` / `}` so the typedef is visible again after the block.
class TypedefRegistry {
public:
    static void clear();
    static void add(const std::string& name, const type::Type& type);
    static bool has(const std::string& name);
    static type::Type lookup(const std::string& name);

    static void addIdentifierShadow(const std::string& name);
    static bool isIdentifierShadow(const std::string& name);
    static void clearIdentifierShadows();
    static void pushIdentifierShadowScope();
    static void popIdentifierShadowScope();

private:
    static std::map<std::string, type::Type>& table();
    static std::vector<std::set<std::string>>& identifierShadowScopes();
};

} // namespace scanner

#endif // TYPEDEFREGISTRY_H_
