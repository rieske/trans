#include "ParseTypeTable.h"

namespace scanner {

ParseTypeTable::ParseTypeTable(NameIntern& intern) :
        intern_ { intern } {
}

void ParseTypeTable::add(const std::string& name, const type::Type& type) {
    const int id = intern_.intern(name);
    if (id == kNoName) {
        return;
    }
    scopes_.back().insert_or_assign(id, type);
}

std::optional<type::Type> ParseTypeTable::lookup(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return std::nullopt;
    }
    auto pending = pending_.find(id);
    if (pending != pending_.end()) {
        return pending->second;
    }
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(id);
        if (found != it->end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

std::optional<int> ParseTypeTable::bindingDepth(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return std::nullopt;
    }
    if (pending_.contains(id)) {
        return static_cast<int>(scopes_.size());
    }
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; --i) {
        if (scopes_[static_cast<std::size_t>(i)].contains(id)) {
            return i;
        }
    }
    return std::nullopt;
}

void ParseTypeTable::addPending(const std::string& name, const type::Type& type) {
    const int id = intern_.intern(name);
    if (id == kNoName) {
        return;
    }
    pending_.insert_or_assign(id, type);
}

void ParseTypeTable::flushPending() {
    for (const auto& entry : pending_) {
        scopes_.back().insert_or_assign(entry.first, entry.second);
    }
    pending_.clear();
}

void ParseTypeTable::clearPending() {
    pending_.clear();
}

bool ParseTypeTable::containsInCurrentScope(std::string_view name) const {
    const int id = intern_.find(name);
    if (id == kNoName) {
        return false;
    }
    return scopes_.back().contains(id);
}

void ParseTypeTable::enterScope() {
    scopes_.push_back({});
    flushPending();
}

void ParseTypeTable::leaveScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

} // namespace scanner
