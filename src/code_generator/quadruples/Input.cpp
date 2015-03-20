#include "Input.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
