#include "ValueEntry.h"

#include <iostream>
#include <sstream>

namespace symbols {

ValueEntry::ValueEntry(std::string name, const type::Type& type, bool tmp, translation_unit::Context context, int index, bool global) :
        name { name },
        type { type },
        context { context },
        index { index },
        temp { tmp },
        global { global }
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

bool ValueEntry::isGlobal() const {
    return global;
}

void ValueEntry::setConstantInitializer(long value) {
    constantInitializer = value;
    multiWordInitializer.reset();
}

std::optional<long> ValueEntry::getConstantInitializer() const {
    return constantInitializer;
}

void ValueEntry::setMultiWordInitializer(std::vector<std::string> words) {
    multiWordInitializer = std::move(words);
    constantInitializer.reset();
}

const std::optional<std::vector<std::string>>& ValueEntry::getMultiWordInitializer() const {
    return multiWordInitializer;
}

} // namespace symbols

