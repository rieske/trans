#include "Mod.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Mod::Mod(Value leftOperand, Value rightOperand, Value result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Mod::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
