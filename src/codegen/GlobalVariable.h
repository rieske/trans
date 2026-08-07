#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <optional>
#include <string>
#include <vector>

#include "Value.h"
#include "types/ObjectAbi.h"

namespace codegen {

// File-scope variable for .data emission. StackMachine records the home in globalHomes and
// a resolve()-only Value via toValue() (not a register cache).
struct GlobalVariable {
    std::string name;
    int sizeInBytes;
    std::string initializerLiteral;
    // When set, emit as a string in .data (db '...') instead of a qword.
    std::optional<std::string> stringInitializer;
    // When set, emit one dq operand per word (brace-initialized structs/arrays).
    std::optional<std::vector<std::string>> multiWordInitializer;
    ValueKind valueType { ValueKind::INTEGRAL };
    // pure extern: declare only (no storage). static: define without exporting.
    bool isExternal { false };
    bool isStatic { false };
    // For sub-word integral loads (emitLoad); default true matches int.
    bool isSigned { true };

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes, isSigned };
    }

    // Scalar float32 .data is 4 bytes; everything else is qword words.
    int dataWidthBytes() const {
        return (valueType == ValueKind::FLOATING && sizeInBytes == 4) ? 4 : 8;
    }

    // Operands for .data emission (dd when dataWidthBytes==4, else dq / .quad).
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
