#include "Div.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Div::Div(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Div::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Div::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " / " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
