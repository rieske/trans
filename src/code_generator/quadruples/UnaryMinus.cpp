#include "UnaryMinus.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

UnaryMinus::UnaryMinus(Value operand, Value result) :
        SingleOperandQuadruple { operand, result }
{
}

void UnaryMinus::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
