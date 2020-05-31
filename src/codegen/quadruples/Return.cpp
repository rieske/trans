#include "Return.h"

#include "../AssemblyGenerator.h"

namespace codegen {

Return::Return(std::string returnSymbolName) :
        returnSymbolName { returnSymbolName }
{
}

void Return::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Return::transfersControl() const {
    return true;
}

std::string Return::getReturnSymbolName() const {
    return returnSymbolName;
}

void Return::print(std::ostream& stream) const {
    stream << "\tRETURN " << getReturnSymbolName() << "\n";
}

} /* namespace code_generator */
