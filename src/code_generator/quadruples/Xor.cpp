#include "Xor.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Xor::Xor(Value leftOperand, Value rightOperand, Value result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Xor::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
