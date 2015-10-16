#include "codegen/quadruples/Call.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

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

void Call::print(std::ostream& stream) const {
    stream << "\tCALL " << getProcedureName() << "\n";
}

} /* namespace code_generator */
