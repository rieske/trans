#include "EnumConstantRegistry.h"

namespace scanner {

void EnumConstantRegistry::add(const std::string& name, long value) {
    table_.insert_or_assign(name, value);
}

bool EnumConstantRegistry::lookup(const std::string& name, long& value) const {
    auto it = table_.find(name);
    if (it == table_.end()) {
        return false;
    }
    value = it->second;
    return true;
}

} // namespace scanner
