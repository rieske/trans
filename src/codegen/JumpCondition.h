#ifndef CODEGEN_JUMP_CONDITION_H_
#define CODEGEN_JUMP_CONDITION_H_

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

} // namespace codegen

#endif // CODEGEN_JUMP_CONDITION_H_
