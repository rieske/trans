#include "Return.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Return::Return(Value returnValue) :
        returnValue { returnValue }
{
}

void Return::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value Return::getReturnValue() const {
    return returnValue;
}

} /* namespace code_generator */
