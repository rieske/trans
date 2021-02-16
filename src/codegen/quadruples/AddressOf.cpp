#include "AddressOf.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

AddressOf::AddressOf(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void AddressOf::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void AddressOf::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := &" << getOperand() << "\n";
}

} // namespace codegen

