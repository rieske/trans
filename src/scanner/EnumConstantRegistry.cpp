#include "EnumConstantRegistry.h"

namespace scanner {

void EnumConstantRegistry::add(const std::string& name, type::IntegerConstant value) {
    scopes_.back().insert_or_assign(name, std::move(value));
}

bool EnumConstantRegistry::lookup(std::string_view name, type::IntegerConstant& value) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            value = found->second;
            return true;
        }
    }
    return false;
}

std::optional<int> EnumConstantRegistry::bindingDepth(std::string_view name) const {
    for (int i = static_cast<int>(scopes_.size()) - 1; i >= 0; --i) {
        if (scopes_[static_cast<std::size_t>(i)].contains(name)) {
            return i;
        }
    }
    return std::nullopt;
}

bool EnumConstantRegistry::contains(std::string_view name) const {
    type::IntegerConstant unused;
    return lookup(name, unused);
}

bool EnumConstantRegistry::containsInCurrentScope(std::string_view name) const {
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

void EnumConstantRegistry::openBody() {
    bodies_.emplace_back();
}

std::optional<type::IntegerConstant> EnumConstantRegistry::nextInCurrentBody() const {
    if (bodies_.empty()) {
        return std::nullopt;
    }
    return bodies_.back().next;
}

void EnumConstantRegistry::recordInCurrentBody(std::string name, type::IntegerConstant value) {
    if (bodies_.empty()) {
        return;
    }
    OpenEnumBody& body = bodies_.back();
    const type::SignedBits v = type::signedValue(value);
    if (!body.hasRange) {
        body.hasRange = true;
        body.min = v;
        body.max = v;
    } else {
        if (v < body.min) {
            body.min = v;
        }
        if (v > body.max) {
            body.max = v;
        }
    }
    body.next = type::nextEnumerator(value);
    body.enumerators.emplace_back(std::move(name), std::move(value));
}

ClosedEnumBody EnumConstantRegistry::closeBody() {
    if (bodies_.empty()) {
        return {};
    }
    OpenEnumBody& body = bodies_.back();
    ClosedEnumBody closed;
    closed.enumerators = std::move(body.enumerators);
    closed.hasRange = body.hasRange;
    closed.min = body.min;
    closed.max = body.max;
    bodies_.pop_back();
    return closed;
}

} // namespace scanner
