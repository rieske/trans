#include "Mul.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Mul::Mul(Value leftOperand, Value rightOperand, Value result) :
        DoubleOperandQuadruple(leftOperand, rightOperand, result)
{
}

void Mul::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
