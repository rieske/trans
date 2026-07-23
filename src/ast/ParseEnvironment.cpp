#include "ParseEnvironment.h"

#include "scanner/EnumConstantRegistry.h"
#include "scanner/TypedefRegistry.h"
#include "types/Type.h"

namespace ast {

type::Type ParseEnvironment::ensureStructTag(const std::string& tag) {
    auto it = structTags_.find(tag);
    if (it != structTags_.end()) {
        return it->second;
    }
    type::Type incomplete = type::incompleteStructure();
    structTags_.insert_or_assign(tag, incomplete);
    return incomplete;
}

void ParseEnvironment::defineTypedef(const std::string& name, type::Type type) {
    typedefs_.insert_or_assign(name, std::move(type));
}

std::optional<type::Type> ParseEnvironment::lookupTypedef(const std::string& name) const {
    auto it = typedefs_.find(name);
    if (it != typedefs_.end()) {
        return it->second;
    }
    // Predefined / process-wide types (e.g. __builtin_va_list) live in TypedefRegistry.
    if (scanner::TypedefRegistry::has(name)) {
        return scanner::TypedefRegistry::lookup(name);
    }
    return std::nullopt;
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
    // Register immediately so later enumerators can fold prior names
    // (e.g. B = A, _SC_IOV_MAX = _SC_UIO_MAXIOV).
    scanner::EnumConstantRegistry::add(name, value);
    frame.second.emplace_back(std::move(name), value);
    frame.first = value + 1;
}

std::vector<std::pair<std::string, long>> ParseEnvironment::endEnumDefinition() {
    if (enumDefinitionStack_.empty()) {
        return {};
    }
    auto result = std::move(enumDefinitionStack_.back().second);
    enumDefinitionStack_.pop_back();
    return result;
}

} // namespace ast
