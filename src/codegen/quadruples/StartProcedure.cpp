#include "StartProcedure.h"

#include "../AssemblyGenerator.h"

namespace codegen {

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

void StartProcedure::print(std::ostream& stream) const {
    stream << "PROC " << getName() << "\n";
}

} /* namespace code_generator */
