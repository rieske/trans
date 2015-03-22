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

void EndScope::print(std::ostream& stream) const {
    stream << "\tEND SCOPE\n";
}

} /* namespace code_generator */
