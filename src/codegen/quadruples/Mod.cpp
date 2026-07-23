#include "Mod.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Mod::Mod(std::string leftOperand, std::string rightOperand, std::string result, bool unsignedMod) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result },
        unsignedMod { unsignedMod }
{
}

void Mod::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Mod::isUnsigned() const {
    return unsignedMod;
}

void Mod::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " % " << getRightOperandName()
            << (unsignedMod ? " (u)\n" : "\n");
}

} // namespace codegen

