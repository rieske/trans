#include "AddressOf.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

AddressOf::AddressOf(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void AddressOf::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
