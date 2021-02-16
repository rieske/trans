#ifndef STORAGESPECIFIER_H_
#define STORAGESPECIFIER_H_

#include <string>

#include "TranslationUnitContextAware.h"

namespace ast {

enum class Storage {
    AUTO, REGISTER, STATIC, EXTERN, TYPEDEF
};

class StorageSpecifier: public TranslationUnitContextAware {
public:
    static StorageSpecifier AUTO(translation_unit::Context context);
    static StorageSpecifier REGISTER(translation_unit::Context context);
    static StorageSpecifier STATIC(translation_unit::Context context);
    static StorageSpecifier EXTERN(translation_unit::Context context);
    static StorageSpecifier TYPEDEF(translation_unit::Context context);

    Storage getStorage() const;

private:
    StorageSpecifier(Storage storage, translation_unit::Context context);

    Storage storage;
};

bool operator==(const StorageSpecifier& lhs, const StorageSpecifier& rhs);
bool operator!=(const StorageSpecifier& lhs, const StorageSpecifier& rhs);

std::string to_string(const StorageSpecifier& specifier);

} // namespace ast

#endif // STORAGESPECIFIER_H_
