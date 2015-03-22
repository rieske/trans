#include "ZeroCompare.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

ZeroCompare::ZeroCompare(std::string symbolName) :
        symbolName { symbolName }
{
}

void ZeroCompare::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string ZeroCompare::getSymbolName() const {
    return symbolName;
}

void ZeroCompare::print(std::ostream& stream) const {
    stream << "\tCMP " << getSymbolName() << ", 0\n";
}

} /* namespace code_generator */
