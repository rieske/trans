#include "Shr.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Shr::Shr(std::string leftOperand, std::string rightOperand, std::string result, bool logical) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result },
        logical { logical }
{
}

void Shr::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Shr::isLogical() const {
    return logical;
}

void Shr::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " >> " << getRightOperandName()
            << (logical ? " (u)\n" : "\n");
}

} // namespace codegen

