#include "Sub.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Sub::Sub(Value leftOperand, Value rightOperand, Value result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Sub::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
