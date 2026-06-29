#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <string>

#include "Value.h"

namespace codegen {

// File-scope variable for .data emission. StackMachine records the home in globalHomes and
// a resolve()-only Value via toValue() (not a register cache).
struct GlobalVariable {
    std::string name;
    int sizeInBytes;
    std::string initializerLiteral;

    Value toValue() const {
        return Value { name, 0, Type::INTEGRAL, sizeInBytes };
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
