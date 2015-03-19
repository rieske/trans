#include "LvalueAssign.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

LvalueAssign::LvalueAssign(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void LvalueAssign::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
