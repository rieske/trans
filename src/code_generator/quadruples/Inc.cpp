#include "Inc.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Inc::Inc(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void Inc::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
