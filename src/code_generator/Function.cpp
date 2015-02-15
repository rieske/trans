#include "Function.h"

namespace code_generator {

Function::Function(std::string name, std::size_t argumentCount) :
        name { name },
        argumentCount { argumentCount }
{
}

std::string Function::getName() const {
    return name;
}

std::size_t Function::getArgumentCount() const {
    return argumentCount;
}

} /* namespace code_generator */

