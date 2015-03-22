#include "Xor.h"

#include "../AssemblyGenerator.h"

namespace codegen {

Xor::Xor(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Xor::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Xor::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " XOR " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
