#include "StartScope.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

StartScope::StartScope(std::vector<Value> values) :
        values { values }
{
}

void StartScope::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::vector<Value> StartScope::getValues() const {
    return values;
}

} /* namespace code_generator */
