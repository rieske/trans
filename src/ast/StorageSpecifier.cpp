#include "StorageSpecifier.h"

#include <stdexcept>

#include "translation_unit/Context.h"

namespace ast {

StorageSpecifier StorageSpecifier::AUTO(translation_unit::Context context) {
    return {Storage::AUTO, context};
}

StorageSpecifier StorageSpecifier::REGISTER(translation_unit::Context context) {
    return {Storage::REGISTER, context};
}

StorageSpecifier StorageSpecifier::STATIC(translation_unit::Context context) {
    return {Storage::STATIC, context};
}

StorageSpecifier StorageSpecifier::EXTERN(translation_unit::Context context) {
    return {Storage::EXTERN, context};
}

StorageSpecifier StorageSpecifier::TYPEDEF(translation_unit::Context context) {
    return {Storage::TYPEDEF, context};
}

StorageSpecifier::StorageSpecifier(Storage storage, translation_unit::Context context) :
        TranslationUnitContextAware { context },
        storage { storage }
{
}

Storage StorageSpecifier::getStorage() const {
    return storage;
}

bool operator==(const StorageSpecifier& lhs, const StorageSpecifier& rhs) {
    return lhs.getStorage() == rhs.getStorage();
}

bool operator!=(const StorageSpecifier& lhs, const StorageSpecifier& rhs) {
    return !(lhs == rhs);
}

std::string to_string(const StorageSpecifier& specifier) {
    switch (specifier.getStorage()) {
    case Storage::AUTO:
        return "auto";
    case Storage::REGISTER:
        return "register";
    case Storage::STATIC:
        return "static";
    case Storage::EXTERN:
        return "extern";
    case Storage::TYPEDEF:
        return "typedef";
    default:
        throw std::runtime_error { "unrecognized StorageSpecifier in StorageSpecifier.cpp" };
    }
}

}
