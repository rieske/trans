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
    Type valueType { Type::INTEGRAL };
    // When set, emit one dq operand per word (brace-initialized structs/arrays).
    std::optional<std::vector<std::string>> multiWordInitializer;

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes };
    }

    // Qword operands for .data emission (shared by Intel dq / gas .quad).
    std::vector<std::string> qwordOperands() const {
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
