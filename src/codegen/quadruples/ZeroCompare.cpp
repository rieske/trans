#include "ZeroCompare.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

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

} // namespace codegen
