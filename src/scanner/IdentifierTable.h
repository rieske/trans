#ifndef IDENTIFIERTABLE_H_
#define IDENTIFIERTABLE_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Ordinary-identifier table: typedef bindings, object types, and object shadows.
// One block-scope stack. File-scope (root) stays until the session dies.
// An object named T hides typedef T without erasing it.
class IdentifierTable {
public:
    void add(const std::string& name, const type::Type& type);
    bool has(const std::string& name) const;
    std::optional<type::Type> tryLookup(const std::string& name) const;

    void addObject(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookupObject(const std::string& name) const;
    void addPendingObject(const std::string& name, const type::Type& type);
    void clearPendingObjects();

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
        std::map<std::string, type::Type> objects;
        std::set<std::string> shadows;
    };

    void flushPendingParameterShadows();
    void flushPendingObjects();

    unsigned revision_ { 0 };
    std::vector<Scope> scopes_ { Scope {} };
    std::set<std::string> pendingParameterShadows_;
    std::map<std::string, type::Type> pendingObjects_;
};

} // namespace scanner

#endif // IDENTIFIERTABLE_H_
