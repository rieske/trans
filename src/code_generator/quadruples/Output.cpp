#include "Output.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Output::Output(Value outputValue) :
        outputValue { outputValue }
{
}

void Output::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value Output::getOutputValue() const {
    return outputValue;
}

} /* namespace code_generator */
