#include "IdentifierTable.h"

namespace scanner {

void IdentifierTable::addTypedef(const std::string& name, const type::Type& type) {
    scopes_.back().typedefs.insert_or_assign(name, type);
    scopes_.back().shadows.erase(name);
    ++revision_;
}

bool IdentifierTable::hasTypedef(std::string_view name) const {
    return lookupTypedef(name).has_value();
}

std::optional<type::Type> IdentifierTable::lookupTypedef(std::string_view name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->typedefs.find(name);
        if (found != it->typedefs.end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

void IdentifierTable::enterScope() {
    scopes_.push_back({});
    ++revision_;
    flushPendingParameterShadows();
}

void IdentifierTable::leaveScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
        ++revision_;
    }
}

void IdentifierTable::addIdentifierShadow(const std::string& name) {
    scopes_.back().shadows.insert(name);
    ++revision_;
}

bool IdentifierTable::isIdentifierShadow(std::string_view name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->shadows.count(name) > 0) {
            return true;
        }
        if (it->typedefs.count(name) > 0) {
            return false;
        }
    }
    return false;
}

void IdentifierTable::addPendingParameterShadow(const std::string& name) {
    pendingParameterShadows_.insert(name);
}

void IdentifierTable::flushPendingParameterShadows() {
    for (const auto& name : pendingParameterShadows_) {
        addIdentifierShadow(name);
    }
    pendingParameterShadows_.clear();
}

void IdentifierTable::clearPendingParameterShadows() {
    pendingParameterShadows_.clear();
}

} // namespace scanner
