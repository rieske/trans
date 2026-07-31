#include "ParseEnvironment.h"

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
    enumDefinitionStack_.emplace_back(0L, std::vector<std::pair<std::string, long>> {});
}

void ParseEnvironment::addEnumerator(std::string name, std::optional<long> explicitValue) {
    if (enumDefinitionStack_.empty()) {
        beginEnumDefinition();
    }
    auto& frame = enumDefinitionStack_.back();
    long value = explicitValue ? *explicitValue : frame.first;
    // Register immediately so later enumerators can fold prior names.
    session_.enums.add(name, value);
    frame.second.emplace_back(std::move(name), value);
    frame.first = value + 1;
}

bool ParseEnvironment::lookupEnumConstant(const std::string& name, long& value) const {
    return session_.enums.lookup(name, value);
}

std::vector<std::pair<std::string, long>> ParseEnvironment::endEnumDefinition() {
    if (enumDefinitionStack_.empty()) {
        return {};
    }
    auto result = std::move(enumDefinitionStack_.back().second);
    enumDefinitionStack_.pop_back();
    return result;
}

std::map<std::string, long> ParseEnvironment::enumConstantsSnapshot() const {
    return session_.enums.entries();
}

} // namespace ast
