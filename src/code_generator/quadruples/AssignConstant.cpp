#include "AssignConstant.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

AssignConstant::AssignConstant(std::string constant, Value result) :
        constant { constant },
        result { result }
{
}

void AssignConstant::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string AssignConstant::getConstant() const{
    return constant;
}

Value AssignConstant::getResult() const{
    return result;
}

} /* namespace code_generator */
