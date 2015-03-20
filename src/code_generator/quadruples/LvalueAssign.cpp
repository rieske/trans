#include "LvalueAssign.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

LvalueAssign::LvalueAssign(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void LvalueAssign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
