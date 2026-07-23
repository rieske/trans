#include "Call.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Call::Call(std::string procedureName, bool indirect, std::string memoryReturnDest) :
        procedureName { procedureName },
        indirect { indirect },
        memoryReturnDest { memoryReturnDest }
{
}

void Call::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Call::getProcedureName() const {
    return procedureName;
}

bool Call::isIndirect() const {
    return indirect;
}

std::string Call::getMemoryReturnDest() const {
    return memoryReturnDest;
}

void Call::print(std::ostream& stream) const {
    if (indirect) {
        stream << "\tCALL *" << getProcedureName();
    } else {
        stream << "\tCALL " << getProcedureName();
    }
    if (!memoryReturnDest.empty()) {
        stream << " sret " << memoryReturnDest;
    }
    stream << "\n";
}


void Call::collectSymbolRefs(SymbolRefs& refs) const {
    refs.isCall = true;
    if (indirect) {
        refs.addUse(procedureName);
    }
    refs.addUse(memoryReturnDest);
}

} // namespace codegen
