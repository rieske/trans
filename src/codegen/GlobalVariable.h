#ifndef GLOBALVARIABLE_H_
#define GLOBALVARIABLE_H_

#include <string>

#include "Value.h"

namespace codegen {

// File-scope variable for preamble emission and StackMachine::globals registration.
struct GlobalVariable {
    std::string name;
    int sizeInBytes;
    std::string initializerLiteral;

    // Stack machine currently models only integral/pointer-sized words as INTEGRAL.
    Value toValue() const {
        return Value { name, 0, Type::INTEGRAL, sizeInBytes, false, true };
    }
};

} // namespace codegen

#endif // GLOBALVARIABLE_H_
