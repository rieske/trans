#include "AssignConstant.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

AssignConstant::AssignConstant(std::string constant, std::string result) :
        constant { constant },
        result { result }
{
}

void AssignConstant::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string AssignConstant::getConstant() const {
    return constant;
}

std::string AssignConstant::getResult() const {
    return result;
}

void AssignConstant::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := " << getConstant() << "\n";
}

} // namespace codegen
