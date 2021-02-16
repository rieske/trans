#include "LvalueAssign.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

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

} // namespace codegen

