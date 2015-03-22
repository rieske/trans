#include "Sub.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Sub::Sub(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Sub::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Sub::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " - " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
