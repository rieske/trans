#include "LvalueAssign.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

LvalueAssign::LvalueAssign(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void LvalueAssign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void LvalueAssign::print(std::ostream& stream) const {
    stream << "\t" << "*" << getResult() << " := " << getOperand() << "\n";
}

} /* namespace code_generator */
