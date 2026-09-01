#ifndef IDENTIFIERTABLE_H_
#define IDENTIFIERTABLE_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Ordinary-identifier table: typedefs, object types, and identifier shadows.
// One block-scope stack. File-scope (root) stays until the session dies.
// Shadows are a token-class hide (typedef_name vs id). addObject does not install one.
class IdentifierTable {
public:
    void addTypedef(const std::string& name, const type::Type& type);
    bool hasTypedef(const std::string& name) const;
    std::optional<type::Type> lookupTypedef(const std::string& name) const;

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
        std::map<std::string, type::Type> typedefs;
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
