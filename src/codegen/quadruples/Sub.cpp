#include "Sub.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

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

} // namespace codegen

