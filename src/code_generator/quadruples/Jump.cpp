#include "Jump.h"

#include "../AssemblyGenerator.h"

namespace code_generator {

Jump::Jump(std::string jumpToLabel, JumpCondition condition) :
        jumpToLabel { jumpToLabel },
        condition { condition }
{
}

void Jump::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

std::string Jump::getLabel() const {
    return jumpToLabel;
}

JumpCondition Jump::getCondition() const {
    return condition;
}

} /* namespace code_generator */
