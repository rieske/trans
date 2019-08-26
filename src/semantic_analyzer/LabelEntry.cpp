#include "LabelEntry.h"

#include <iostream>

namespace semantic_analyzer {

LabelEntry::LabelEntry(std::string name) :
        name { name } {
}

void LabelEntry::print() const {
    std::cout << "\t" << name << "\t\ttemp\t0\t\tlabel" << std::endl;
}

std::string LabelEntry::getName() const {
    return name;
}

} /* namespace semantic_analyzer */
