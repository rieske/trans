#include "LvalueAssign.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

LvalueAssign::LvalueAssign(std::string operand, std::string result, int accessSizeBytes) :
        SingleOperandQuadruple { operand, result },
        accessSizeBytes { accessSizeBytes }
{
}

void LvalueAssign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void LvalueAssign::print(std::ostream& stream) const {
    stream << "\t" << "*" << getResult() << " := " << getOperand()
            << " [" << accessSizeBytes << "]\n";
}

void LvalueAssign::collectSymbolRefs(SymbolRefs& refs) const {
    // Store through address: both the value and the address are uses.
    refs.addUse(getOperand());
    refs.addUse(getResult());
}

} // namespace codegen
