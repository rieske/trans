#include "EnumConstantRegistry.h"

namespace scanner {

void EnumConstantRegistry::add(const std::string& name, type::IntegerConstant value) {
    table_.insert_or_assign(name, std::move(value));
}

bool EnumConstantRegistry::lookup(const std::string& name, type::IntegerConstant& value) const {
    auto it = table_.find(name);
    if (it == table_.end()) {
        return false;
    }
    value = it->second;
    return true;
}

bool EnumConstantRegistry::contains(const std::string& name) const {
    return table_.find(name) != table_.end();
}

} // namespace scanner
