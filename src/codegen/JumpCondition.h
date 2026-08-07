#ifndef CODEGEN_JUMP_CONDITION_H_
#define CODEGEN_JUMP_CONDITION_H_

namespace codegen {

enum class JumpCondition {
    IF_EQUAL,
    IF_NOT_EQUAL,
    IF_BELOW,
    IF_ABOVE,
    IF_BELOW_OR_EQUAL,
    IF_ABOVE_OR_EQUAL,
    UNCONDITIONAL
};

} // namespace codegen

#endif // CODEGEN_JUMP_CONDITION_H_
