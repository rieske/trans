#include "Or.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Or::Or(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Or::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

}
/* namespace code_generator */
