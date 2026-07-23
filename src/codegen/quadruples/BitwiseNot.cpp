#include "BitwiseNot.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

BitwiseNot::BitwiseNot(std::string operand, std::string result) :
        SingleOperandQuadruple { operand, result }
{
}

void BitwiseNot::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void BitwiseNot::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := ~" << getOperand() << "\n";
}

} // namespace codegen
