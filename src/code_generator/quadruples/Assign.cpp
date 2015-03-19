#include "Assign.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Assign::Assign(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void Assign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
