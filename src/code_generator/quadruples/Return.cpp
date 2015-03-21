#include "Return.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
