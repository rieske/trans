#include "TypedefRegistry.h"

#include <stdexcept>

namespace scanner {

std::map<std::string, type::Type>& TypedefRegistry::table() {
    static std::map<std::string, type::Type> typedefs;
    return typedefs;
}

std::vector<std::set<std::string>>& TypedefRegistry::identifierShadowScopes() {
    static std::vector<std::set<std::string>> scopes;
    return scopes;
}

void TypedefRegistry::clear() {
    table().clear();
    identifierShadowScopes().clear();
}

void TypedefRegistry::add(const std::string& name, const type::Type& type) {
    table().insert_or_assign(name, type);
    // A new typedef binding wins over any prior object shadow of the same name.
    for (auto& scope : identifierShadowScopes()) {
        scope.erase(name);
    }
}

bool TypedefRegistry::has(const std::string& name) {
    return table().count(name) > 0;
}

type::Type TypedefRegistry::lookup(const std::string& name) {
    auto it = table().find(name);
    if (it == table().end()) {
        throw std::runtime_error { "unknown typedef name: " + name };
    }
    return it->second;
}

void TypedefRegistry::addIdentifierShadow(const std::string& name) {
    if (identifierShadowScopes().empty()) {
        identifierShadowScopes().push_back({});
    }
    identifierShadowScopes().back().insert(name);
}

bool TypedefRegistry::isIdentifierShadow(const std::string& name) {
    for (const auto& scope : identifierShadowScopes()) {
        if (scope.count(name) > 0) {
            return true;
        }
    }
    return false;
}

void TypedefRegistry::clearIdentifierShadows() {
    identifierShadowScopes().clear();
}

void TypedefRegistry::pushIdentifierShadowScope() {
    identifierShadowScopes().push_back({});
}

void TypedefRegistry::popIdentifierShadowScope() {
    if (!identifierShadowScopes().empty()) {
        identifierShadowScopes().pop_back();
    }
}

} // namespace scanner
