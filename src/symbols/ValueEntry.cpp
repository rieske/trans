#include "ValueEntry.h"

#include <cassert>

namespace symbols {

ValueEntry::ValueEntry(std::string name, const type::Type& type, translation_unit::Context context, int index,
        Storage storage) :
        name { name },
        type { type },
        context { context },
        index { index },
        storage { storage }
{
}

type::Type ValueEntry::getType() const {
    return type;
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

void ValueEntry::setGlobalInitializer(GlobalInitializer init) {
    initializer_ = std::move(init);
}

void ValueEntry::setType(const type::Type& newType) {
    type = newType;
}

} // namespace symbols
