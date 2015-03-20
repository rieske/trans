#include "Output.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Output::Output(std::string outputSymbolName) :
        outputSymbolName { outputSymbolName }
{
}

void Output::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Output::getOutputSymbolName() const {
    return outputSymbolName;
}

} /* namespace code_generator */
