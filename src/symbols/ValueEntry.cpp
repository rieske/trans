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

const type::Type& ValueEntry::getType() const {
    return type;
}

void ValueEntry::refineType(const type::Type& refined) {
    type = refined;
}

void ValueEntry::setContext(translation_unit::Context newContext) {
    context = std::move(newContext);
}

void ValueEntry::markFunctionDefined() {
    functionDefined_ = true;
}

bool ValueEntry::isFunctionDefined() const {
    return functionDefined_;
}

void ValueEntry::applyFunctionSpecs(bool isInline, bool isNoreturn, bool isExtern) {
    if (isInline) {
        inline_ = true;
    } else {
        externalNonInline_ = true;
    }
    if (isNoreturn) {
        noreturn_ = true;
    }
    if (isExtern) {
        functionExtern_ = true;
    }
}

bool ValueEntry::isNoreturn() const {
    return noreturn_;
}

bool ValueEntry::providesExternalDefinition() const {
    if (isStatic()) {
        return false;
    }
    return externalNonInline_ || !inline_ || functionExtern_;
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

