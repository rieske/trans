#ifndef IDENTIFIERTABLE_H_
#define IDENTIFIERTABLE_H_

#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "types/Type.h"

namespace scanner {

// Ordinary-identifier table: typedefs, object types, and identifier shadows.
// One block-scope stack. File-scope (root) stays until the session dies.
// Shadows are a token-class hide (typedef_name vs id). addObject does not install one.
class IdentifierTable {
public:
    void addTypedef(const std::string& name, const type::Type& type);
    bool hasTypedef(std::string_view name) const;
    std::optional<type::Type> lookupTypedef(std::string_view name) const;

    void addObject(const std::string& name, const type::Type& type);
    std::optional<type::Type> lookupObject(std::string_view name) const;
    void addPendingObject(const std::string& name, const type::Type& type);
    void clearPendingObjects();

    void enterScope();
    void leaveScope();

    void addIdentifierShadow(const std::string& name);
    bool isIdentifierShadow(std::string_view name) const;

    void addPendingParameterShadow(const std::string& name);
    void clearPendingParameterShadows();
    unsigned revision() const { return revision_; }

private:
    struct Scope {
        std::map<std::string, type::Type, std::less<>> typedefs;
        std::map<std::string, type::Type, std::less<>> objects;
        std::set<std::string, std::less<>> shadows;
    };

    void flushPendingParameterShadows();
    void flushPendingObjects();

    unsigned revision_ { 0 };
    std::vector<Scope> scopes_ { Scope {} };
    std::set<std::string> pendingParameterShadows_;
    std::map<std::string, type::Type, std::less<>> pendingObjects_;
};

} // namespace scanner

#endif // IDENTIFIERTABLE_H_
