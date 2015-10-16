#include "codegen/quadruples/Return.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Return::Return(std::string returnSymbolName) :
        returnSymbolName { returnSymbolName }
{
}

void Return::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Return::getReturnSymbolName() const {
    return returnSymbolName;
}

void Return::print(std::ostream& stream) const {
    stream << "\tRETURN " << getReturnSymbolName() << "\n";
}

} /* namespace code_generator */
