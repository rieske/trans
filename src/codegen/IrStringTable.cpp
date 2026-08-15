#include "IrStringTable.h"

#include <stdexcept>
#include <string>

namespace codegen {

int IrStringTable::intern(std::string_view text) {
    if (text.empty()) {
        return kNoSymbol;
    }
    auto existing = index_.find(text);
    if (existing != index_.end()) {
        return existing->second;
    }
    const int id = static_cast<int>(names_.size());
    names_.emplace_back(text);
    index_.emplace(names_.back(), id);
    return id;
}

int IrStringTable::find(std::string_view text) const {
    if (text.empty()) {
        return kNoSymbol;
    }
    auto existing = index_.find(text);
    if (existing == index_.end()) {
        return kNoSymbol;
    }
    return existing->second;
}

int IrStringTable::require(std::string_view text) const {
    const int id = find(text);
    if (id < 0) {
        throw std::logic_error { "IrStringTable::require: missing `" + std::string(text) + "`" };
    }
    return id;
}

const std::string& IrStringTable::get(int id) const {
    if (id < 0 || id >= static_cast<int>(names_.size())) {
        throw std::logic_error { "IrStringTable::get: invalid id" };
    }
    return names_[static_cast<std::size_t>(id)];
}

} // namespace codegen
