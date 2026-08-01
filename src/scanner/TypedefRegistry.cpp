#include "TypedefRegistry.h"

namespace scanner {

void TypedefRegistry::add(const std::string& name, const type::Type& type) {
    // Last-wins on redefinition; no product diagnostic for incompatible re-typedef yet.
    table_.insert_or_assign(name, type);
    // A new typedef binding wins over any prior object shadow of the same name.
    for (auto& scope : identifierShadowScopes_) {
        scope.erase(name);
    }
}

bool TypedefRegistry::has(const std::string& name) const {
    return table_.count(name) > 0;
}

std::optional<type::Type> TypedefRegistry::tryLookup(const std::string& name) const {
    auto it = table_.find(name);
    if (it == table_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void TypedefRegistry::addIdentifierShadow(const std::string& name) {
    if (identifierShadowScopes_.empty()) {
        identifierShadowScopes_.push_back({});
    }
    identifierShadowScopes_.back().insert(name);
}

bool TypedefRegistry::isIdentifierShadow(const std::string& name) const {
    for (const auto& scope : identifierShadowScopes_) {
        if (scope.count(name) > 0) {
            return true;
        }
    }
    return false;
}

void TypedefRegistry::pushIdentifierShadowScope() {
    identifierShadowScopes_.push_back({});
}

void TypedefRegistry::popIdentifierShadowScope() {
    if (!identifierShadowScopes_.empty()) {
        identifierShadowScopes_.pop_back();
    }
}

void TypedefRegistry::addPendingParameterShadow(const std::string& name) {
    pendingParameterShadows_.insert(name);
}

void TypedefRegistry::flushPendingParameterShadows() {
    for (const auto& name : pendingParameterShadows_) {
        addIdentifierShadow(name);
    }
    pendingParameterShadows_.clear();
}

void TypedefRegistry::clearPendingParameterShadows() {
    pendingParameterShadows_.clear();
}

} // namespace scanner
