#include "ObjectTypeRegistry.h"

namespace scanner {

void ObjectTypeRegistry::pushScope() {
    scopes_.push_back({});
}

void ObjectTypeRegistry::popScope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

void ObjectTypeRegistry::add(const std::string& name, const type::Type& type) {
    if (scopes_.empty()) {
        scopes_.push_back({});
    }
    scopes_.back().insert_or_assign(name, type);
}

std::optional<type::Type> ObjectTypeRegistry::lookup(const std::string& name) const {
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

void ObjectTypeRegistry::addPending(const std::string& name, const type::Type& type) {
    pending_.insert_or_assign(name, type);
}

void ObjectTypeRegistry::flushPending() {
    for (const auto& entry : pending_) {
        add(entry.first, entry.second);
    }
    pending_.clear();
}

void ObjectTypeRegistry::clearPending() {
    pending_.clear();
}

} // namespace scanner
