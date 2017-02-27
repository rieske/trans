#include "ValueCompare.h"

#include "../AssemblyGenerator.h"

namespace codegen {

ValueCompare::ValueCompare(std::string leftSymbolName, std::string rightSymbolName) :
        leftSymbolName { leftSymbolName },
        rightSymbolName { rightSymbolName }
{
}

void ValueCompare::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

void ValueCompare::print(std::ostream& stream) const {
    stream << "\tCMP " << getLeftSymbolName() << ", " << getRightSymbolName() << "\n";
}

std::string ValueCompare::getLeftSymbolName() const {
    return leftSymbolName;
}

std::string ValueCompare::getRightSymbolName() const {
    return rightSymbolName;
}

} /* namespace code_generator */
