#include "Add.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Add::Add(Value leftOperand, Value rightOperand, Value result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Add::generateCode(AssemblyGenerator& generator) const{
    generator.generateCodeFor(*this);
}

} /* namespace code_generator */
