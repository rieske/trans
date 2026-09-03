#include "StorageSpecifier.h"

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
        storage { storage },
        context { context }
{
}

Storage StorageSpecifier::getStorage() const {
    return storage;
}

translation_unit::Context StorageSpecifier::getContext() const {
    return context;
}

} // namespace ast

