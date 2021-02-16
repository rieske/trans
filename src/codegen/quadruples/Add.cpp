#include "Add.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Add::Add(std::string leftOperand, std::string rightOperand, std::string result) :
        DoubleOperandQuadruple { leftOperand, rightOperand, result }
{
}

void Add::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void Add::print(std::ostream& stream) const {
    stream << "\t" << getResultName() << " := " << getLeftOperandName() << " + " << getRightOperandName() << "\n";
}

} // namespace codegen

