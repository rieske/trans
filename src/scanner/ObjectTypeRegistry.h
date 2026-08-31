#ifndef OBJECTTYPEREGISTRY_H_
#define OBJECTTYPEREGISTRY_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Object types are block-scoped. File-scope (root) stays until the session dies.
class ObjectTypeRegistry {
public:
    void enterScope();
    void leaveScope();
    void add(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookup(const std::string& name) const;
    void addPending(const std::string& name, const type::Type& type);
    void clearPending();

private:
    void flushPending();

    std::vector<std::map<std::string, type::Type>> scopes_ { {} };
    std::map<std::string, type::Type> pending_;
};

} // namespace scanner

#endif
