#include "codegen/quadruples/Label.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Label::Label(std::string name) :
        name { name }
{
}

void Label::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Label::getName() const {
    return name;
}

void Label::print(std::ostream& stream) const {
    stream << getName() << ":\n";
}

} /* namespace code_generator */
