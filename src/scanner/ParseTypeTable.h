#ifndef PARSETYPETABLE_H_
#define PARSETYPETABLE_H_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Parse-time types of ordinary identifiers (objects, functions, parameters).
// Hops with LexicalSession braces. Not token-class; not IR homes.
class ParseTypeTable {
public:
    void add(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookup(std::string_view name) const;
    void addPending(const std::string& name, const type::Type& type);
    void clearPending();

    void enterScope();
    void leaveScope();

private:
    void flushPending();

    std::vector<std::map<std::string, type::Type, std::less<>>> scopes_ { {} };
    std::map<std::string, type::Type, std::less<>> pending_;
};

} // namespace scanner

#endif // PARSETYPETABLE_H_
