#include "StackMachine.h"
#include "StackMachineInternal.h"

#include "InstructionSet.h"

namespace codegen {

void StackMachine::finishInstruction() {
    // Drop register-held expression temps whose last use was this instruction
    // (or earlier). Do not spill: their stack slot may already be reused.
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!reg->containsUnstoredValue()) {
            continue;
        }
        Value* value = reg->getValue();
        const int lastUse = value->getLastUseOrdinal();
        if (lastUse >= 0 && lastUse <= instructionOrdinal) {
            reg->free();
        }
    }
    ++instructionOrdinal;
}

void StackMachine::label(std::string name) {
    spillGeneralPurposeRegisters();
    assembly.label(instructionSet->label(name));
}

void StackMachine::jump(JumpCondition jumpCondition, std::string label) {
    // Spill before every jump (including conditional). Otherwise a value live in a
    // register on only one predecessor of a join is never written to its stack home
    // (git strbuf_grow: formal `sb` used after `if (!sb->alloc)`).
    // x86 mov does not clobber flags, so spilling after cmp is safe.
    spillGeneralPurposeRegisters();
    switch (jumpCondition) {
    case JumpCondition::IF_EQUAL:
        assembly << instructionSet->je(label);
        break;
    case JumpCondition::IF_NOT_EQUAL:
        assembly << instructionSet->jne(label);
        break;
    case JumpCondition::IF_ABOVE:
        assembly << instructionSet->jg(label);
        break;
    case JumpCondition::IF_BELOW:
        assembly << instructionSet->jl(label);
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        assembly << instructionSet->jge(label);
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        assembly << instructionSet->jle(label);
        break;
    case JumpCondition::IF_ABOVE_U:
        assembly << instructionSet->ja(label);
        break;
    case JumpCondition::IF_BELOW_U:
        assembly << instructionSet->jb(label);
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL_U:
        assembly << instructionSet->jae(label);
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL_U:
        assembly << instructionSet->jbe(label);
        break;
    case JumpCondition::UNCONDITIONAL:
    default:
        assembly << instructionSet->jmp(label);
    }
}

} // namespace codegen
