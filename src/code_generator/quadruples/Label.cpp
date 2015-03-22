#include "Label.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

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
