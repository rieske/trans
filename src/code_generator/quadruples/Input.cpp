#include "Input.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Input::Input(Value inputValue) :
        inputValue { inputValue }
{
}

void Input::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value Input::getInputValue() const {
    return inputValue;
}

} /* namespace code_generator */
