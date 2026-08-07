#include "ParseEnvironment.h"

#include <stdexcept>

#include "types/Type.h"

namespace ast {

ParseEnvironment::ParseEnvironment(scanner::LexicalSession& session) :
        session_ { session } {
}

type::Type ParseEnvironment::ensureStructTag(const std::string& tag) {
    auto it = structTags_.find(tag);
    if (it != structTags_.end()) {
        return it->second;
    }
    type::Type incomplete = type::incompleteStructure();
    structTags_.emplace(tag, incomplete);
    return incomplete;
}

void ParseEnvironment::defineTypedef(const std::string& name, type::Type type) {
    session_.typedefs.add(name, type);
}

std::optional<type::Type> ParseEnvironment::lookupTypedef(const std::string& name) const {
    return session_.typedefs.tryLookup(name);
}

void ParseEnvironment::registerInitializedDeclaration(
        const DeclarationSpecifiers& specs,
        const std::vector<std::unique_ptr<InitializedDeclarator>>& declarators) {
    if (specs.isTypedef()) {
        // Incomplete reduction (typedef with no type-specs): no alias to register.
        // Soft-return rather than throw; pin via ParseEnvironment unit test.
        if (specs.getTypeSpecifiers().empty()) {
            return;
        }
        auto baseType = specs.getResolvedType();
        for (const auto& declarator : declarators) {
            type::Type aliased = declarator->getFundamentalType(baseType);
            defineTypedef(declarator->getName(), aliased);
        }
        return;
    }
    for (const auto& declarator : declarators) {
        const std::string& name = declarator->getName();
        if (lookupTypedef(name)) {
            session_.typedefs.addIdentifierShadow(name);
        }
    }
}

void ParseEnvironment::beginEnumDefinition() {
    enumNextValueStack_.push_back(0L);
}

void ParseEnvironment::addEnumerator(std::string name, std::optional<long> explicitValue) {
    // First enumerator opens the auto-increment window (no separate CSNB begin).
    if (enumNextValueStack_.empty()) {
        beginEnumDefinition();
    }
    long value = explicitValue ? *explicitValue : enumNextValueStack_.back();
    // Any redefinition of an enumerator name is a constraint violation (C),
    // including same-value and names introduced by other enums / structs.
    long existing = 0;
    if (session_.enums.lookup(name, existing)) {
        throw std::runtime_error { "redefinition of enumerator `" + name + "`" };
    }
    // Register immediately so later enumerators can fold prior names.
    session_.enums.add(name, value);
    enumNextValueStack_.back() = value + 1;
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name, long& value) const {
    return session_.enums.lookup(name, value);
}

void ParseEnvironment::endEnumDefinition() {
    // Idempotent: empty enum bodies never call addEnumerator (still OK to end).
    if (!enumNextValueStack_.empty()) {
        enumNextValueStack_.pop_back();
    }
}

std::map<std::string, long> ParseEnvironment::enumConstantsSnapshot() const {
    return session_.enums.entries();
}

void ParseEnvironment::maybeRegisterParameterShadow(const std::string& name) {
    if (name.empty() || !lookupTypedef(name)) {
        return;
    }
    session_.typedefs.addPendingParameterShadow(name);
}

} // namespace ast
