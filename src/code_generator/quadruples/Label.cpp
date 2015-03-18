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

std::string Label::getName() {
    return name;
}

} /* namespace code_generator */
