#include "ValueEntry.h"

#include <cassert>

namespace symbols {

ValueEntry::ValueEntry(std::string name, const type::Type& type, translation_unit::Context context, int index,
        Storage storage, std::string sourceName) :
        name { std::move(name) },
        sourceName_ { std::move(sourceName) },
        type { type },
        context { context },
        index { index },
        storage { storage }
{
}

type::Type ValueEntry::getType() const {
    return type;
}

void ValueEntry::refineType(const type::Type& refined) {
    type = refined;
}

translation_unit::Context ValueEntry::getContext() const {
    return context;
}

int ValueEntry::getIndex() const {
    return index;
}

const std::string& ValueEntry::getName() const {
    return name;
}

const std::string& ValueEntry::sourceName() const {
    return sourceName_;
}

bool ValueEntry::isGlobal() const {
    return storage != Storage::Automatic;
}

bool ValueEntry::isStatic() const {
    return storage == Storage::Static;
}

bool ValueEntry::isExtern() const {
    return storage == Storage::Extern;
}

void ValueEntry::promoteExternToDefinition() {
    assert(storage == Storage::Extern);
    storage = Storage::Global;
}

bool ValueEntry::hasDefiningInitializer() const {
    return definingInitializer;
}

void ValueEntry::markDefiningInitializer() {
    definingInitializer = true;
}

void ValueEntry::setStaticInit(std::vector<StaticInitValue> words) {
    staticInitWords = std::move(words);
}

const std::vector<StaticInitValue>& ValueEntry::staticInit() const {
    return staticInitWords;
}

} // namespace symbols

