#include "Output.h"

#include "../AssemblyGenerator.h"

namespace codegen {

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

void Output::print(std::ostream& stream) const {
    stream << "\tOUTPUT " << getOutputSymbolName() << "\n";
}

} /* namespace code_generator */
