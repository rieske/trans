#include "LabelEntry.h"

#include <iostream>

namespace code_generator {

LabelEntry::LabelEntry(std::string name) :
        name { name } {
}

LabelEntry::~LabelEntry() {
}

void LabelEntry::print() const {
    std::cout << "\t" << name << "\t\ttemp\t0\t\tlabel" << std::endl;
}

std::string LabelEntry::getName() const {
    return name;
}

} /* namespace code_generator */
