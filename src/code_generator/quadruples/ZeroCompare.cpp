#include "ZeroCompare.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

ZeroCompare::ZeroCompare(Value argument) :
        argument { argument }
{
}

void ZeroCompare::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value ZeroCompare::getArgument() const {
    return argument;
}

} /* namespace code_generator */
