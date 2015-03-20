#include "UnaryMinus.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

UnaryMinus::UnaryMinus(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void UnaryMinus::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
