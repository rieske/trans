#include "Div.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Div::Div(std::string leftOperand, std::string rightOperand, std::string result, bool unsignedDiv) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result },
        unsignedDiv { unsignedDiv }
{
}

void Div::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Div::isUnsigned() const {
    return unsignedDiv;
}

void Div::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " / " << getRightOperandName()
            << (unsignedDiv ? " (u)\n" : "\n");
}

} // namespace codegen
