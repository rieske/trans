#include "IdentifierTable.h"

namespace scanner {

IdentifierTable::IdentifierTable(NameIntern& intern) :
        intern_ { intern } {
}

void IdentifierTable::addTypedef(const std::string& name, const type::Type& type) {
    const int id = intern_.intern(name);
    if (id == kNoName) {
        return;
    }
    scopes_.back().typedefs.insert_or_assign(id, type);
    scopes_.back().shadows.erase(id);
    ++revision_;
}

bool IdentifierTable::hasTypedef(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return false;
    }
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->typedefs.find(id) != it->typedefs.end()) {
            return true;
        }
    }
    return false;
}

std::optional<type::Type> IdentifierTable::lookupTypedef(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return std::nullopt;
    }
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->typedefs.find(id);
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
    const int id = intern_.intern(name);
    if (id == kNoName) {
        return;
    }
    scopes_.back().shadows.insert(id);
    ++revision_;
}

bool IdentifierTable::isIdentifierShadow(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return false;
    }
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->shadows.count(id) > 0) {
            return true;
        }
        if (it->typedefs.count(id) > 0) {
            return false;
        }
    }
    return false;
}

void IdentifierTable::addPendingParameterShadow(const std::string& name) {
    const int id = intern_.intern(name);
    if (id == kNoName) {
        return;
    }
    pendingParameterShadows_.insert(id);
}

void IdentifierTable::flushPendingParameterShadows() {
    for (int id : pendingParameterShadows_) {
        scopes_.back().shadows.insert(id);
        ++revision_;
    }
    pendingParameterShadows_.clear();
}

void IdentifierTable::clearPendingParameterShadows() {
    pendingParameterShadows_.clear();
}

} // namespace scanner
