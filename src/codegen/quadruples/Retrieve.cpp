#include "Retrieve.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Retrieve::Retrieve(std::string resultName, bool memoryReturn) :
        resultName { std::move(resultName) },
        memoryReturn { memoryReturn }
{
}

void Retrieve::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Retrieve::getResultName() const {
    return resultName;
}

bool Retrieve::isMemoryReturn() const {
    return memoryReturn;
}

void Retrieve::print(std::ostream& stream) const {
    stream << "\tRETRIEVE " << getResultName() << (memoryReturn ? " sret" : "") << "\n";
}


void Retrieve::collectSymbolRefs(SymbolRefs& refs) const {
    refs.addDef(resultName);
}

} // namespace codegen

