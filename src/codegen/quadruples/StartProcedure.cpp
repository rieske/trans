#include "StartProcedure.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

StartProcedure::StartProcedure(std::string name, std::vector<Value> values, std::vector<Value> arguments,
        bool variadic, bool isStatic, bool memoryReturn) :
        name { name },
        values { values },
        arguments { arguments },
        variadic { variadic },
        staticFunction { isStatic },
        memoryReturn { memoryReturn }
{
}

void StartProcedure::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool StartProcedure::isLabel() const {
    return true;
}

std::string StartProcedure::getName() const {
    return name;
}

std::vector<Value> StartProcedure::getValues() const {
    return values;
}

std::vector<Value> StartProcedure::getArguments() const {
    return arguments;
}

bool StartProcedure::isVariadic() const {
    return variadic;
}

bool StartProcedure::isStatic() const {
    return staticFunction;
}

bool StartProcedure::hasMemoryReturn() const {
    return memoryReturn;
}

void StartProcedure::print(std::ostream& stream) const {
    stream << "PROC " << (staticFunction ? "static " : "") << getName()
           << (memoryReturn ? " sret" : "") << (variadic ? " ..." : "") << "\n";
}

} // namespace codegen
