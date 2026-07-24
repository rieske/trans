#ifndef JUMP_H_
#define JUMP_H_

#include <string>

#include "Quadruple.h"

namespace codegen {

enum class JumpCondition {
    IF_EQUAL,
    IF_NOT_EQUAL,
    // Signed relational (jg/jl/jge/jle).
    IF_BELOW,
    IF_ABOVE,
    IF_BELOW_OR_EQUAL,
    IF_ABOVE_OR_EQUAL,
    // Unsigned relational (ja/jb/jae/jbe) - size_t overflow checks, pointers.
    IF_BELOW_U,
    IF_ABOVE_U,
    IF_BELOW_OR_EQUAL_U,
    IF_ABOVE_OR_EQUAL_U,
    UNCONDITIONAL
};

class Jump: public Quadruple {
public:
    Jump(std::string jumpToLabel, JumpCondition condition = JumpCondition::UNCONDITIONAL);
    virtual ~Jump() = default;

    void generateCode(AssemblyGenerator& generator) const override;

    bool transfersControl() const override;

    std::string getLabel() const;
    JumpCondition getCondition() const;

private:
    void print(std::ostream& stream) const override;

    std::string jumpToLabel;
    JumpCondition condition;
};

} // namespace codegen

#endif // JUMP_H_
