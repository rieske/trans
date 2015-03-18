#include "Dereference.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Dereference::Dereference(Value argument, Value result) :
        argument { argument },
        result { result }
{
}

void Dereference::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

Value Dereference::getArgument() const {
    return argument;
}

Value Dereference::getResult() const {
    return result;
}

} /* namespace code_generator */

