#include "Retrieve.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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

} /* namespace code_generator */
