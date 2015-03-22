#include "StartScope.h"

#include "../AssemblyGenerator.h"

namespace codegen {

StartScope::StartScope(std::vector<Value> values, std::vector<Value> arguments) :
        values { values },
        arguments { arguments }
{
}

void StartScope::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::vector<Value> StartScope::getValues() const {
    return values;
}

std::vector<Value> StartScope::getArguments() const {
    return arguments;
}

void StartScope::print(std::ostream& stream) const {
    stream << "\tBEGIN SCOPE\n";
}

} /* namespace code_generator */
