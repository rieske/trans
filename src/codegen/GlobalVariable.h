#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include "Value.h"
#include "symbols/GlobalInitializer.h"
#include "types/ObjectAbi.h"
#include "types/SysVClass.h"

namespace codegen {

enum class ObjectEmission {
    DefineExternal,
    DefineInternal,
    Reference
};

// File-scope variable for .data emission. StackMachine records the home in globalHomes and
// a resolve()-only Value via toValue() (not a register cache).
struct GlobalVariable {
    std::string name;
    int sizeInBytes { 0 };
    std::optional<symbols::GlobalInitializer> initializer;
    ValueKind valueType { ValueKind::INTEGRAL };
    type::sysv::Classification classification {};
    ObjectEmission emission { ObjectEmission::DefineExternal };
    // For sub-word integral loads (emitLoad); default true matches int.
    bool isSigned { true };

    bool emitAsDword() const { return dataWidthBytes() == 4; }

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes, isSigned, classification };
    }

    // Scalar float32 .data is 4 bytes; everything else is qword words.
    int dataWidthBytes() const {
        return (valueType == ValueKind::FLOATING && sizeInBytes == 4) ? 4 : 8;
    }

    std::vector<symbols::DataWord> dataWords() const {
        const int words = type::object_abi::dataWords(sizeInBytes);
        const int n = words > 0 ? words : 1;
        std::vector<symbols::DataWord> operands;
        operands.reserve(static_cast<std::size_t>(n));
        if (initializer) {
            std::visit([&](const auto& arm) {
                using T = std::decay_t<decltype(arm)>;
                if constexpr (std::is_same_v<T, symbols::MultiWordInit>) {
                    operands = arm.words;
                } else if constexpr (std::is_same_v<T, symbols::AddressInit>
                        || std::is_same_v<T, symbols::ConstantInit>) {
                    operands.push_back(arm);
                }
            }, *initializer);
        }
        if (operands.empty()) {
            operands.emplace_back(symbols::ConstantInit { 0 });
        }
        while (static_cast<int>(operands.size()) < n) {
            operands.emplace_back(symbols::ConstantInit { 0 });
        }
        return operands;
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
