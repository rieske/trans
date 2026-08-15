#include "LabelEntry.h"

namespace symbols {

LabelEntry::LabelEntry(std::string name) :
        name { name } {
}

const std::string& LabelEntry::getName() const {
    return name;
}

} // namespace symbols
