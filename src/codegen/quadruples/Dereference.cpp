#include "Dereference.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Dereference::Dereference(std::string operand, std::string lvalue, std::string result, int accessSizeBytes,
        bool isSigned) :
        SingleOperandQuadruple { operand, result },
        lvalue { lvalue },
        accessSizeBytes { accessSizeBytes },
        isSigned { isSigned }
{
}

void Dereference::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Dereference::getLvalue() const {
    return lvalue;
}

void Dereference::print(std::ostream& stream) const {
    // Include lvalue name: packFrameValues / lastUseOrdinal scan print text.
    // Omitting it made the pointer temp look dead during the load, so its stack
    // slot was reused (e.g. *p <<= k, multi-word *dst = *src).
    stream << "\t" << getResult() << " := *" << getOperand()
            << " [" << accessSizeBytes << (isSigned ? "s" : "u") << "] => "
            << lvalue << "\n";
}


void Dereference::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addUse(getOperand());
    refs.addDef(getResult());
    refs.addUse(lvalue);
    refs.addDef(lvalue);
}

} // namespace codegen
