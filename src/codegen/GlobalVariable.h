#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <string>
#include <vector>

#include "Value.h"
#include "symbols/StaticInit.h"
#include "types/ObjectAbi.h"

namespace codegen {

enum class ObjectEmission {
    DefineExternal,
    DefineInternal,
    Reference
};

// .data object. Home lives in globalHomes; toValue() is resolve-only (not register-cached).
struct GlobalVariable {
    std::string name;
    int sizeInBytes;
    int alignBytes { 1 };
    Type valueType { Type::INTEGRAL };
    type::sysv::Classification classification {};
    std::vector<symbols::StaticInitValue> initValues;
    ObjectEmission emission { ObjectEmission::DefineExternal };

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes, classification };
    }

    bool emitAsDword() const {
        return isSseFloat32(toValue());
    }

    std::vector<symbols::StaticInitValue> initValuesOrZeros() const {
        if (!initValues.empty()) {
            return initValues;
        }
        const int words = type::object_abi::dataWords(sizeInBytes);
        return std::vector<symbols::StaticInitValue>(
                static_cast<std::size_t>(words > 0 ? words : 1), symbols::StaticWord {});
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
