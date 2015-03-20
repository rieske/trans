#include "Assign.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Assign::Assign(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void Assign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
