#include "Shl.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Shl::Shl(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Shl::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Shl::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " << " << getRightOperandName() << "\n";
}

} // namespace codegen

