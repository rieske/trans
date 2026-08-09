#ifndef TYPEDEFREGISTRY_H_
#define TYPEDEFREGISTRY_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Typedef names for one TU. FA uses has(); TokenStream uses identifier shadows.
class TypedefRegistry {
public:
    void add(const std::string& name, const type::Type& type);
    bool has(const std::string& name) const;
    std::optional<type::Type> tryLookup(const std::string& name) const;

    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(const std::string& name) const;
    void pushIdentifierShadowScope();
    void popIdentifierShadowScope();

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
