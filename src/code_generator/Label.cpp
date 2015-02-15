#include "Label.h"

namespace code_generator {

Label::Label(std::string name) :
        name { name }
{
}

std::string Label::getName() const {
    return name;
}

} /* namespace code_generator */
