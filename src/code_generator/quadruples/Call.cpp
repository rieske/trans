#include "Call.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Call::Call(std::string procedureName) :
        procedureName { procedureName }
{
}

void Call::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Call::getProcedureName() const {
    return procedureName;
}

} /* namespace code_generator */
