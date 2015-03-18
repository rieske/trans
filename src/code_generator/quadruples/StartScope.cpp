#include "StartScope.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

StartScope::StartScope(int scopeSize) :
        scopeSize { scopeSize }
{
}

void StartScope::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

int StartScope::getScopeSize() const {
    return scopeSize;
}

} /* namespace code_generator */
