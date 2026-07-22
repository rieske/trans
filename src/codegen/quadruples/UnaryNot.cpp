#include "UnaryNot.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

UnaryNot::UnaryNot(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void UnaryNot::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void UnaryNot::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := ~" << getOperand() << "\n";
}

} // namespace codegen

