#include "Inc.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Inc::Inc(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void Inc::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
