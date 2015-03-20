#include "Dereference.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */

