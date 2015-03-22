#include "AddressOf.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
