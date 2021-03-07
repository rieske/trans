#include "ValueEntry.h"

#include <iostream>
#include <sstream>

namespace semantic_analyzer {

ValueEntry::ValueEntry(std::string name, const type::Type& type, bool tmp, translation_unit::Context context, int index) :
        name { name },
        type { type },
        context { context },
        index { index },
        temp { tmp }
{
}

type::Type ValueEntry::getType() const {
    return type;
}

std::string ValueEntry::to_string() const {
    std::stringstream str;
    str << "\t" << name << "\t" << (temp ? "temp" : "") << "\t" << index << "\t" << type.to_string() << std::endl;
    return str.str();
}

translation_unit::Context ValueEntry::getContext() const {
    return context;
}

int ValueEntry::getIndex() const {
    return index;
}

std::string ValueEntry::getName() const {
    return name;
}

} // namespace semantic_analyzer

