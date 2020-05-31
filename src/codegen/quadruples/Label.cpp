#include "Label.h"

#include "../AssemblyGenerator.h"

namespace codegen {

Label::Label(std::string name) :
        name { name }
{
}

void Label::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Label::isLabel() const {
    return true;
}

std::string Label::getName() const {
    return name;
}

void Label::print(std::ostream& stream) const {
    stream << getName() << ":\n";
}

} /* namespace code_generator */
