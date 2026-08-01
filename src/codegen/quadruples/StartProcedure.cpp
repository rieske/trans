#include "StartProcedure.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

StartProcedure::StartProcedure(std::string name, std::vector<Value> values, std::vector<Value> arguments) :
        name { name },
        values { values },
        arguments { arguments }
{
}

void StartProcedure::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool StartProcedure::isLabel() const {
    return true;
}

bool StartProcedure::definesProcedure(std::string& nameOut) const {
    nameOut = name;
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

void StartProcedure::print(std::ostream& stream) const {
    stream << "PROC " << getName() << "\n";
}

} // namespace codegen

