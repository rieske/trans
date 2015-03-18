#include "AddressOf.h"

#include "../AssemblyGenerator.h"
#include "../Value.h"

namespace code_generator {

AddressOf::AddressOf(Value argument, Value result) :
        argument { argument },
        result { result }
{
}

void AddressOf::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value AddressOf::getArgument() const {
    return argument;
}

Value AddressOf::getResult() const {
    return result;
}

} /* namespace code_generator */
