#include "LabelEntry.h"
#include "util/Logger.h"
#include "util/LogManager.h"

#include <iostream>

namespace semantic_analyzer {

LabelEntry::LabelEntry(std::string name) :
        name { name } {
}

std::string LabelEntry::getName() const {
    return name;
}

} /* namespace semantic_analyzer */
