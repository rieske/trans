#include "EndScope.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

EndScope::EndScope(int scopeSize) :
        scopeSize { scopeSize }
{
}

void EndScope::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

int EndScope::getScopeSize() const {
    return scopeSize;
}

} /* namespace code_generator */
