#include "Retrieve.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Retrieve::Retrieve(Value result) :
        result { result }
{
}

void Retrieve::generateCode(AssemblyGenerator& generator) const{
    generator.generateCodeFor(*this);
}

Value Retrieve::getResult() const {
    return result;
}

} /* namespace code_generator */
