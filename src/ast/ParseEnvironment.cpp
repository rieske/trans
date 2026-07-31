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
    // Sole channel for typedef registration (lexer + lookup).
    session_.typedefs.add(name, type);
}

std::optional<type::Type> ParseEnvironment::lookupTypedef(const std::string& name) const {
    return session_.typedefs.tryLookup(name);
}

void ParseEnvironment::beginEnumDefinition() {
    nextEnumeratorValue_ = 0L;
}

void ParseEnvironment::addEnumerator(std::string name, std::optional<long> explicitValue) {
    // First enumerator opens the auto-increment window (no separate CSNB begin).
    if (!nextEnumeratorValue_) {
        beginEnumDefinition();
    }
    long value = explicitValue ? *explicitValue : *nextEnumeratorValue_;
    // Any redefinition of an enumerator name is a constraint violation (C),
    // including same-value and names introduced by other enums / structs.
    long existing = 0;
    if (session_.enums.lookup(name, existing)) {
        throw std::runtime_error { "redefinition of enumerator `" + name + "`" };
    }
    // Register immediately so later enumerators can fold prior names.
    session_.enums.add(name, value);
    nextEnumeratorValue_ = value + 1;
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name, long& value) const {
    return session_.enums.lookup(name, value);
}

void ParseEnvironment::endEnumDefinition() {
    // Idempotent: empty enum bodies never call addEnumerator (still OK to end).
    nextEnumeratorValue_.reset();
}

std::map<std::string, long> ParseEnvironment::enumConstantsSnapshot() const {
    return session_.enums.entries();
}

} // namespace ast
