#include "TypedefRegistry.h"

namespace scanner {

void TypedefRegistry::add(const std::string& name, const type::Type& type) {
    scopes_.back().bindings.insert_or_assign(name, type);
    scopes_.back().shadows.erase(name);
    ++revision_;
}

bool TypedefRegistry::has(const std::string& name) const {
    return tryLookup(name).has_value();
}

std::optional<type::Type> TypedefRegistry::tryLookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->bindings.find(name);
        if (found != it->bindings.end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

void TypedefRegistry::enterScope() {
    scopes_.push_back({});
    ++revision_;
    flushPendingParameterShadows();
}

void TypedefRegistry::leaveScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
        ++revision_;
    }
}

void TypedefRegistry::addIdentifierShadow(const std::string& name) {
    scopes_.back().shadows.insert(name);
    ++revision_;
}

bool TypedefRegistry::isIdentifierShadow(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->shadows.count(name) > 0) {
            return true;
        }
        if (it->bindings.count(name) > 0) {
            return false;
        }
    }
    return false;
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
