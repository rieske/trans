#include "ValueEntry.h"

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

bool ValueEntry::isExtern() const {
    return storage == Storage::Extern;
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

