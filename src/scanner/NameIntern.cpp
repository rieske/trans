#include "NameIntern.h"

namespace scanner {

int NameIntern::intern(std::string_view text) {
    if (text.empty()) {
        return kNoName;
    }
    auto existing = index_.find(text);
    if (existing != index_.end()) {
        return existing->second;
    }
    const int id = static_cast<int>(index_.size());
    index_.emplace(std::string { text }, id);
    return id;
}

int NameIntern::find(std::string_view text) const {
    if (text.empty()) {
        return kNoName;
    }
    auto existing = index_.find(text);
    if (existing == index_.end()) {
        return kNoName;
    }
    return existing->second;
}

} // namespace scanner
