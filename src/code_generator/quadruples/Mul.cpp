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

void Mul::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " * " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
