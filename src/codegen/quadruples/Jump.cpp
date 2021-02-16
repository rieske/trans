#include "Jump.h"

#include "codegen/AssemblyGenerator.h"

namespace codegen {

Jump::Jump(std::string jumpToLabel, JumpCondition condition) :
        jumpToLabel { jumpToLabel },
        condition { condition }
{
}

void Jump::generateCode(AssemblyGenerator& generator) const {
    generator.generateCodeFor(*this);
}

bool Jump::transfersControl() const {
    return true;
}

std::string Jump::getLabel() const {
    return jumpToLabel;
}

JumpCondition Jump::getCondition() const {
    return condition;
}

void Jump::print(std::ostream& stream) const {
    stream << "\t";
    switch (getCondition()) {
    case JumpCondition::IF_EQUAL:
        stream << "JE ";
        break;
    case JumpCondition::IF_NOT_EQUAL:
        stream << "JNE ";
        break;
    case JumpCondition::IF_ABOVE:
        stream << "JA ";
        break;
    case JumpCondition::IF_BELOW:
        stream << "JB ";
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        stream << "JAE ";
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        stream << "JBE ";
        break;
    case JumpCondition::UNCONDITIONAL:
        default:
        stream << "GOTO ";
    }
    stream << getLabel() << "\n";
}

} // namespace codegen

