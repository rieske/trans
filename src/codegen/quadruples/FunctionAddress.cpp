#include "FunctionAddress.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

FunctionAddress::FunctionAddress(std::string functionName, std::string result) :
        SingleOperandQuadruple { functionName, result }
{
}

void FunctionAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void FunctionAddress::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := &" << getOperand() << " (function)\n";
}

void FunctionAddress::collectSymbolRefs(SymbolRefs& refs) const {
    // Function label is not a live Value; only the result temp is defined.
    refs.addDef(getResult());
}

} // namespace codegen
