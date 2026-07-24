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
}

std::optional<long> ValueEntry::getConstantInitializer() const {
    return constantInitializer;
}

void ValueEntry::setStringInitializer(std::string value) {
    stringInitializer = std::move(value);
}

bool ValueEntry::hasStringInitializer() const {
    return stringInitializer.has_value();
}

const std::string& ValueEntry::getStringInitializer() const {
    return *stringInitializer;
}

void ValueEntry::setAddressInitializer(std::string symbolName) {
    addressInitializer = std::move(symbolName);
}

bool ValueEntry::hasAddressInitializer() const {
    return addressInitializer.has_value();
}

const std::string& ValueEntry::getAddressInitializer() const {
    return *addressInitializer;
}

void ValueEntry::setMultiWordInitializer(std::vector<std::string> words) {
    multiWordInitializer = std::move(words);
}

bool ValueEntry::hasMultiWordInitializer() const {
    return multiWordInitializer.has_value();
}

const std::vector<std::string>& ValueEntry::getMultiWordInitializer() const {
    return *multiWordInitializer;
}

void ValueEntry::setType(const type::Type& newType) {
    type = newType;
}

void ValueEntry::setExternal(bool value) {
    external = value;
}

bool ValueEntry::isExternal() const {
    return external;
}

void ValueEntry::setStaticStorage(bool value) {
    staticStorage = value;
}

bool ValueEntry::isStaticStorage() const {
    return staticStorage;
}

} // namespace symbols

