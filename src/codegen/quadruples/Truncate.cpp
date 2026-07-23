#include "Truncate.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Truncate::Truncate(std::string operandName, int sizeBytes, bool isSigned) :
        operandName { operandName },
        sizeBytes { sizeBytes },
        signedExtend { isSigned }
{
}

void Truncate::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Truncate::getOperandName() const {
    return operandName;
}

void Truncate::print(std::ostream& stream) const {
    stream << "\tTRUNC" << (signedExtend ? "S" : "Z") << sizeBytes * 8
           << " " << getOperandName() << "\n";
}


void Truncate::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(operandName);
    refs.addDef(operandName);
}

} // namespace codegen
