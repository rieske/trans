#ifndef STORAGESPECIFIER_H_
#define STORAGESPECIFIER_H_

#include <string>

#include "../translation_unit/Context.h"

namespace ast {

enum class Storage {
    AUTO, REGISTER, STATIC, EXTERN, TYPEDEF
};

class StorageSpecifier {
public:
    static StorageSpecifier AUTO(translation_unit::Context context);
    static StorageSpecifier REGISTER(translation_unit::Context context);
    static StorageSpecifier STATIC(translation_unit::Context context);
    static StorageSpecifier EXTERN(translation_unit::Context context);
    static StorageSpecifier TYPEDEF(translation_unit::Context context);

    ~StorageSpecifier() = default;

    Storage getStorage() const;
    translation_unit::Context getContext() const;

private:
    StorageSpecifier(Storage storage, translation_unit::Context context);

    Storage storage;
    translation_unit::Context context;
};

bool operator==(const StorageSpecifier& lhs, const StorageSpecifier& rhs);
bool operator!=(const StorageSpecifier& lhs, const StorageSpecifier& rhs);

std::string to_string(const StorageSpecifier& specifier);

}

#endif /* STORAGESPECIFIER_H_ */
