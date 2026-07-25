#include "FunctionAddress.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

FunctionAddress::FunctionAddress(std::string functionName, std::string result) :
        SingleOperandQuadruple { std::move(functionName), std::move(result) }
{
}

void FunctionAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void FunctionAddress::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := &" << getOperand() << " (function)\n";
}

} // namespace codegen
