#include "EndProcedure.h"

#include "../AssemblyGenerator.h"

namespace codegen {

EndProcedure::EndProcedure(std::string name) :
        name { name }
{
}

void EndProcedure::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string EndProcedure::getName() const {
    return name;
}

void EndProcedure::print(std::ostream& stream) const {
    stream << "ENDPROC " << getName() << "\n";
}

} /* namespace code_generator */
