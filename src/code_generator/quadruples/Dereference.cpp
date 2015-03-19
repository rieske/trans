#include "Dereference.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Dereference::Dereference(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void Dereference::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */

