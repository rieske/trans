#include "Retrieve.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Retrieve::Retrieve(std::string resultName) :
        resultName { resultName }
{
}

void Retrieve::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Retrieve::getResultName() const {
    return resultName;
}

void Retrieve::print(std::ostream& stream) const {
    stream << "\tRETRIEVE " << getResultName() << "\n";
}

} // namespace codegen

