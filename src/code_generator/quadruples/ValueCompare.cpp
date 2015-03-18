#include "ValueCompare.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

ValueCompare::ValueCompare(Value leftArg, Value rightArg) :
        leftArg { leftArg },
        rightArg { rightArg }
{
}

void ValueCompare::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value ValueCompare::getLeftArg() const {
    return leftArg;
}

Value ValueCompare::getRightArg() const {
    return rightArg;
}

} /* namespace code_generator */
