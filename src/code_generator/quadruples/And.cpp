#include "And.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

And::And(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void And::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void And::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " AND " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
