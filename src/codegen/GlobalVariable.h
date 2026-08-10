#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <optional>
#include <string>
#include <vector>

#include "Value.h"
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
    std::string initializerLiteral;
    Type valueType { Type::INTEGRAL };
    type::sysv::Classification classification {};
    // When set, emit one operand per word (brace-initialized structs/arrays).
    std::optional<std::vector<std::string>> multiWordInitializer;
    ObjectEmission emission { ObjectEmission::DefineExternal };

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes, classification };
    }

    bool emitAsDword() const {
        return isSseFloat32(toValue());
    }

    // Data operands for .data emission (dd when emitAsDword, else dq / .quad).
    std::vector<std::string> dataOperands() const {
        if (multiWordInitializer && !multiWordInitializer->empty()) {
            return *multiWordInitializer;
        }
        const int words = type::object_abi::dataWords(sizeInBytes);
        std::vector<std::string> operands;
        operands.reserve(static_cast<std::size_t>(words > 0 ? words : 1));
        operands.push_back(initializerLiteral);
        for (int i = 1; i < words; ++i) {
            operands.push_back("0");
        }
        return operands;
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
