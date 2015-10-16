#include "codegen/quadruples/UnaryMinus.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

UnaryMinus::UnaryMinus(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void UnaryMinus::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void UnaryMinus::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := -" << getOperand() << "\n";
}

} /* namespace code_generator */
