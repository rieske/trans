#include "EnumConstantRegistry.h"

namespace scanner {

void EnumConstantRegistry::add(const std::string& name, type::IntegerConstant value) {
    scopes_.back().insert_or_assign(name, std::move(value));
}

bool EnumConstantRegistry::lookup(const std::string& name, type::IntegerConstant& value) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            value = found->second;
            return true;
        }
    }
    return false;
}

bool EnumConstantRegistry::contains(const std::string& name) const {
    type::IntegerConstant unused;
    return lookup(name, unused);
}

bool EnumConstantRegistry::containsInCurrentScope(const std::string& name) const {
    return scopes_.back().find(name) != scopes_.back().end();
}

void EnumConstantRegistry::enterScope() {
    scopes_.push_back({});
}

void EnumConstantRegistry::leaveScope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

} // namespace scanner
