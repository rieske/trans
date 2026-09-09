#include "ParseTypeTable.h"

namespace scanner {

void ParseTypeTable::add(const std::string& name, const type::Type& type) {
    scopes_.back().insert_or_assign(name, type);
}

std::optional<type::Type> ParseTypeTable::lookup(std::string_view name) const {
    auto pending = pending_.find(name);
    if (pending != pending_.end()) {
        return pending->second;
    }
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

std::optional<int> ParseTypeTable::bindingDepth(std::string_view name) const {
    if (pending_.contains(name)) {
        return static_cast<int>(scopes_.size());
    }
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; --i) {
        if (scopes_[static_cast<std::size_t>(i)].contains(name)) {
            return i;
        }
    }
    return std::nullopt;
}

void ParseTypeTable::addPending(const std::string& name, const type::Type& type) {
    pending_.insert_or_assign(name, type);
}

void ParseTypeTable::flushPending() {
    for (const auto& entry : pending_) {
        add(entry.first, entry.second);
    }
    pending_.clear();
}

void ParseTypeTable::clearPending() {
    pending_.clear();
}

bool ParseTypeTable::containsInCurrentScope(std::string_view name) const {
    return scopes_.back().contains(name);
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
