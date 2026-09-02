#ifndef ENUMCONSTANTREGISTRY_H_
#define ENUMCONSTANTREGISTRY_H_

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "types/IntegerConstant.h"

namespace scanner {

// Enumerators are block-scoped. File-scope (root) stays until the session dies.
// add() is last-wins in the current frame; redefinition diagnostics belong at PE/SA.
class EnumConstantRegistry {
public:
    void add(const std::string& name, type::IntegerConstant value);
    bool lookup(std::string_view name, type::IntegerConstant& value) const;
    bool contains(std::string_view name) const;
    bool containsInCurrentScope(std::string_view name) const;

    void enterScope();
    void leaveScope();

private:
    std::vector<std::map<std::string, type::IntegerConstant, std::less<>>> scopes_ { {} };
};

} // namespace scanner

#endif // ENUMCONSTANTREGISTRY_H_
