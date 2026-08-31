#ifndef TYPEDEFREGISTRY_H_
#define TYPEDEFREGISTRY_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// One block-scope stack. Each frame holds typedef bindings and object shadows.
// File-scope (root) stays until the session dies.
// An object named T hides typedef T without erasing it.
class TypedefRegistry {
public:
    void add(const std::string& name, const type::Type& type);
    bool has(const std::string& name) const;
    std::optional<type::Type> tryLookup(const std::string& name) const;

    void enterScope();
    void leaveScope();

    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(const std::string& name) const;

    void addPendingParameterShadow(const std::string& name);
    void clearPendingParameterShadows();
    unsigned revision() const { return revision_; }

private:
    struct Scope {
        std::map<std::string, type::Type> bindings;
        std::set<std::string> shadows;
    };

    void flushPendingParameterShadows();

    unsigned revision_ { 0 };
    std::vector<Scope> scopes_ { Scope {} };
    std::set<std::string> pendingParameterShadows_;
};

} // namespace scanner

#endif // TYPEDEFREGISTRY_H_
