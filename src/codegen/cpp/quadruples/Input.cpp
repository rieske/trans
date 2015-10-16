#include "codegen/quadruples/Input.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Input::Input(std::string inputSymbolName) :
        inputSymbolName { inputSymbolName }
{
}

void Input::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Input::getInputSymbolName() const {
    return inputSymbolName;
}

void Input::print(std::ostream& stream) const {
    stream << "\tINPUT " << getInputSymbolName() << "\n";
}

} /* namespace code_generator */
