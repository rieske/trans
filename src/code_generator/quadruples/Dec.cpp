#include "Dec.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Dec::Dec(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void Dec::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
