#ifndef OBJECTTYPEREGISTRY_H_
#define OBJECTTYPEREGISTRY_H_

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

class ObjectTypeRegistry {
public:
    void pushScope();
    void popScope();
    void add(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookup(const std::string& name) const;
    void addPending(const std::string& name, const type::Type& type);
    void flushPending();
    void clearPending();

private:
    std::vector<std::map<std::string, type::Type>> scopes_;
    std::map<std::string, type::Type> pending_;
};

} // namespace scanner

#endif
