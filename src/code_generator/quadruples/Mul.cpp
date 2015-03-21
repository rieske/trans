#include "Mul.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Mul::Mul(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple(leftOperand, rightOperand, result)
{
}

void Mul::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
