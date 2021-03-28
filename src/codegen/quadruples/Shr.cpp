#include "Shr.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Shr::Shr(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Shr::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Shr::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " >> " << getRightOperandName() << "\n";
}

} // namespace codegen

