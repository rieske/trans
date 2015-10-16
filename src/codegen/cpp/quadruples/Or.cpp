#include "codegen/quadruples/Or.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Or::Or(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Or::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Or::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " OR " << getRightOperandName() << "\n";
}

}
/* namespace code_generator */
