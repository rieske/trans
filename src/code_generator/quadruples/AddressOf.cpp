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

} /* namespace code_generator */
