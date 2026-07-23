#include "EnumConstantRegistry.h"

namespace scanner {

std::map<std::string, long>& EnumConstantRegistry::table() {
    static std::map<std::string, long> constants;
    return constants;
}

void EnumConstantRegistry::clear() {
    table().clear();
}

void EnumConstantRegistry::add(const std::string& name, long value) {
    table().insert_or_assign(name, value);
}

bool EnumConstantRegistry::lookup(const std::string& name, long& value) {
    auto it = table().find(name);
    if (it == table().end()) {
        return false;
    }
    value = it->second;
    return true;
}

} // namespace scanner
