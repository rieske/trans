#include "AssignLabelAddress.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

AssignLabelAddress::AssignLabelAddress(std::string label, std::string result) :
        label { std::move(label) },
        result { std::move(result) }
{
}

void AssignLabelAddress::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string AssignLabelAddress::getLabel() const {
    return label;
}

std::string AssignLabelAddress::getResult() const {
    return result;
}

void AssignLabelAddress::print(std::ostream& stream) const {
    stream << "\t" << getResult() << " := &" << getLabel() << "\n";
}

} // namespace codegen
