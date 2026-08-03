#include "ValueEntry.h"

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

void ValueEntry::setGlobalInitializer(GlobalInitializer init) {
    initializer_ = std::move(init);
}

void ValueEntry::setConstantInitializer(long value) {
    setGlobalInitializer(ConstantInit { value });
}

void ValueEntry::setStringInitializer(std::string value) {
    setGlobalInitializer(StringInit { std::move(value) });
}

void ValueEntry::setAddressInitializer(std::string symbolName) {
    setGlobalInitializer(AddressInit { std::move(symbolName) });
}

void ValueEntry::setMultiWordInitializer(std::vector<std::string> words) {
    setGlobalInitializer(MultiWordInit { std::move(words) });
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
