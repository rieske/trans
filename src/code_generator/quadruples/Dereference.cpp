#include "Dereference.h"

#include "../AssemblyGenerator.h"

namespace codegen {

Dereference::Dereference(std::string operand, std::string lvalue, std::string result) :
        SingleOperandQuadruple { operand, result },
        lvalue { lvalue }
{
}

void Dereference::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Dereference::getLvalue() const {
    return lvalue;
}

void Dereference::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := *" << getOperand() << "\n";
}

} /* namespace code_generator */

