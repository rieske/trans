#include "codegen/quadruples/Mod.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Mod::Mod(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Mod::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Mod::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " % " << getRightOperandName() << "\n";
}

} /* namespace code_generator */
