#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <optional>
#include <string>
#include <vector>

#include "Value.h"

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
    Type valueType { Type::INTEGRAL };
    // pure extern: declare only (no storage). static: define without exporting.
    bool isExternal { false };
    bool isStatic { false };
    // For sub-word integral loads (emitLoad); default true matches int.
    bool isSigned { true };

    Value toValue() const {
        return Value { name, 0, valueType, sizeInBytes, isSigned };
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
