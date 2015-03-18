#ifndef JUMP_H_
#define JUMP_H_

#include <string>

#include "Quadruple.h"

namespace code_generator {

enum class JumpCondition {
    IF_EQUAL, IF_NOT_EQUAL, IF_BELOW, IF_ABOVE, IF_BELOW_OR_EQUAL, IF_ABOVE_OR_EQUAL, UNCONDITIONAL
};

class Jump: public Quadruple {
public:
    Jump(std::string jumpToLabel, JumpCondition condition = JumpCondition::UNCONDITIONAL);
    virtual ~Jump() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    std::string getLabel() const;
    JumpCondition getCondition() const;

private:
    std::string jumpToLabel;
    JumpCondition condition;
};

} /* namespace code_generator */

#endif /* JUMP_H_ */
