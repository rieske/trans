#include "StartProcedure.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

StartProcedure::StartProcedure(std::string name) :
        name { name }
{
}

void StartProcedure::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string StartProcedure::getName() const {
    return name;
}

} /* namespace code_generator */
