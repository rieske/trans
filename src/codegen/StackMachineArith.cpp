#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "types/ObjectAbi.h"

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

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = resolve(leftSymbolName);
    auto& rightSymbol = resolve(rightSymbolName);

    // Usual arithmetic conversions for relational ops: if either side is floating,
    // promote integral operands to double bits before comparing. Integer cmp of
    // double bits vs bare int 1 made `sample_rate <= 1` always false for normal
    // positive doubles (git bitmapPseudoMerge.sampleRate bounds check).
    // IEEE-754 bit patterns order as signed integers for non-NaN values, so after
    // conversion a GPR cmp + jl/jle matches C relational semantics.
    const bool floating = leftSymbol.getValueKind() == ValueKind::FLOATING
            || rightSymbol.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        // Park both values in xmm first so GPR temps cannot clobber each other.
        auto loadToXmm = [&](Value& v, const char* xmm) {
            Register& tmp = get64BitRegister();
            if (residesInMemory(v)) {
                emitLoad(v, tmp);
            } else {
                assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
            }
            if (v.getValueKind() == ValueKind::INTEGRAL) {
                assembly << (std::string("cvtsi2sd ") + xmm + ", " + tmp.getName());
            } else {
                assembly << (std::string("movq ") + xmm + ", " + tmp.getName());
            }
        };
        loadToXmm(leftSymbol, "xmm0");
        loadToXmm(rightSymbol, "xmm1");
        Register& leftReg = get64BitRegister();
        Register& rightReg = get64BitRegisterExcluding(leftReg);
        assembly << (std::string("movq ") + leftReg.getName() + ", xmm0");
        assembly << (std::string("movq ") + rightReg.getName() + ", xmm1");
        assembly << instructionSet->cmp(leftReg, rightReg);
        return;
    }

    // Sub-word stack slots must not be compared as qwords (high bits may be dirty
    // after stores through narrower pointers). Load via emitLoad first.
    auto needsRegisterCompare = [](const Value& v) {
        return v.getValueKind() == ValueKind::INTEGRAL && v.getSizeInBytes() > 0 && v.getSizeInBytes() < MACHINE_WORD_SIZE;
    };
    if (needsRegisterCompare(leftSymbol) || needsRegisterCompare(rightSymbol)
            || (residesInMemory(leftSymbol) && residesInMemory(rightSymbol))) {
        Register& leftReg = residesInMemory(leftSymbol)
                ? assignRegisterTo(leftSymbol)
                : leftSymbol.getAssignedRegister();
        Register& rightReg = residesInMemory(rightSymbol)
                ? assignRegisterExcluding(rightSymbol, leftReg)
                : rightSymbol.getAssignedRegister();
        assembly << instructionSet->cmp(leftReg, rightReg);
        return;
    }
    if (residesInMemory(leftSymbol)) {
        assembly << instructionSet->cmp(memoryOperand(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (residesInMemory(rightSymbol)) {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), memoryOperand(rightSymbol));
    } else {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = resolve(symbolName);
    if (residesInMemory(symbol)) {
        // Load with emitLoad so sub-word slots are narrowed before cmp.
        if (symbol.getValueKind() == ValueKind::INTEGRAL && symbol.getSizeInBytes() > 0
                && symbol.getSizeInBytes() < MACHINE_WORD_SIZE) {
            Register& reg = assignRegisterTo(symbol);
            assembly << instructionSet->cmp(reg, 0);
            return;
        }
        assembly << instructionSet->cmp(memoryOperand(symbol), 0);
    } else {
        assembly << instructionSet->cmp(symbol.getAssignedRegister(), 0);
    }
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    // IEEE float/double: flip sign bit. Integer two's-complement neg corrupts the bit
    // pattern (git / C: d = -1.5 must be -1.5, not neg(0x3ff8000000000000)).
    if (operand.getValueKind() == ValueKind::FLOATING) {
        Register& resultRegister = residesInMemory(operand)
                ? get64BitRegister()
                : get64BitRegisterExcluding(operand.getAssignedRegister());
        if (residesInMemory(operand)) {
            emitLoad(operand, resultRegister);
        } else {
            assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
        }
        Register& mask = get64BitRegisterExcluding(resultRegister);
        assembly << instructionSet->mov("0x8000000000000000", mask);
        // xor_(operand, result) => result ^= operand
        assembly << instructionSet->xor_(mask, resultRegister);
        bindResult(resultRegister, resolve(resultName));
        return;
    }
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    }
}

void StackMachine::bitwiseNot(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    }
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->xor_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->xor_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->or_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->or_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->and_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

// Emit SSE2 binary op for double-precision values held as GPR bit patterns.
// Converts INTEGRAL operands with cvtsi2sd first.
void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    Register& resultRegister = get64BitRegister();

    const bool floating = leftOperand.getValueKind() == ValueKind::FLOATING || rightOperand.getValueKind() == ValueKind::FLOATING
            || result.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        auto toXmm = [&](Value& v, const char* xmm) {
            Register& tmp = get64BitRegister();
            if (residesInMemory(v)) {
                emitLoad(v, tmp);
            } else {
                assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
            }
            if (v.getValueKind() == ValueKind::INTEGRAL) {
                assembly << (std::string("cvtsi2sd ") + xmm + ", " + tmp.getName());
            } else {
                assembly << (std::string("movq ") + xmm + ", " + tmp.getName());
            }
        };
        toXmm(leftOperand, "xmm0");
        toXmm(rightOperand, "xmm1");
        assembly << "addsd xmm0, xmm1";
        assembly << ("movq " + resultRegister.getName() + ", xmm0");
        bindResult(resultRegister, result);
        return;
    }

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    // Sub-word stack slots must not be used as qword memory operands: a store
    // through T* only writes the type width (git: unsigned len via &len, then
    // hex[len] / base+len). Load+narrow first.
    auto subwordIntegral = [](const Value& v) {
        return v.getValueKind() == ValueKind::INTEGRAL && v.getSizeInBytes() > 0
                && v.getSizeInBytes() < MACHINE_WORD_SIZE;
    };
    if (residesInMemory(rightOperand)) {
        if (subwordIntegral(rightOperand)) {
            Register& rightReg = get64BitRegisterExcluding(resultRegister);
            emitLoad(rightOperand, rightReg);
            assembly << instructionSet->add(rightReg, resultRegister);
        } else {
            assembly << instructionSet->add(memoryOperand(rightOperand), resultRegister);
        }
    } else {
        assembly << instructionSet->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, result);
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    Register& resultRegister = get64BitRegister();

    const bool floating = leftOperand.getValueKind() == ValueKind::FLOATING || rightOperand.getValueKind() == ValueKind::FLOATING
            || result.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        auto toXmm = [&](Value& v, const char* xmm) {
            Register& tmp = get64BitRegister();
            if (residesInMemory(v)) {
                emitLoad(v, tmp);
            } else {
                assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
            }
            if (v.getValueKind() == ValueKind::INTEGRAL) {
                assembly << (std::string("cvtsi2sd ") + xmm + ", " + tmp.getName());
            } else {
                assembly << (std::string("movq ") + xmm + ", " + tmp.getName());
            }
        };
        toXmm(leftOperand, "xmm0");
        toXmm(rightOperand, "xmm1");
        assembly << "subsd xmm0, xmm1";
        assembly << ("movq " + resultRegister.getName() + ", xmm0");
        bindResult(resultRegister, result);
        return;
    }

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    auto subwordIntegral = [](const Value& v) {
        return v.getValueKind() == ValueKind::INTEGRAL && v.getSizeInBytes() > 0
                && v.getSizeInBytes() < MACHINE_WORD_SIZE;
    };
    if (residesInMemory(rightOperand)) {
        if (subwordIntegral(rightOperand)) {
            Register& rightReg = get64BitRegisterExcluding(resultRegister);
            emitLoad(rightOperand, rightReg);
            assembly << instructionSet->sub(rightReg, resultRegister);
        } else {
            assembly << instructionSet->sub(memoryOperand(rightOperand), resultRegister);
        }
    } else {
        assembly << instructionSet->sub(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, result);
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    const bool floating = leftOperand.getValueKind() == ValueKind::FLOATING || rightOperand.getValueKind() == ValueKind::FLOATING
            || result.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        Register& resultRegister = get64BitRegister();
        auto toXmm = [&](Value& v, const char* xmm) {
            Register& tmp = get64BitRegister();
            if (residesInMemory(v)) {
                emitLoad(v, tmp);
            } else {
                assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
            }
            if (v.getValueKind() == ValueKind::INTEGRAL) {
                assembly << (std::string("cvtsi2sd ") + xmm + ", " + tmp.getName());
            } else {
                assembly << (std::string("movq ") + xmm + ", " + tmp.getName());
            }
        };
        toXmm(leftOperand, "xmm0");
        toXmm(rightOperand, "xmm1");
        assembly << "mulsd xmm0, xmm1";
        assembly << ("movq " + resultRegister.getName() + ", xmm0");
        bindResult(resultRegister, result);
        return;
    }

    if (result.getValueKind() != ValueKind::INTEGRAL) {
        throw std::runtime_error{"multiplication of non integers is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    // imul writes RDX:RAX; spill RDX if it holds a live value (e.g. pointer for *p *= ...)
    storeRegisterValue(registers->getRemainderRegister());
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->imul(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->imul(rightOperand.getAssignedRegister());
    }
    bindResult(multiplicationRegister, result);
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool unsignedDiv) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    const bool floating = leftOperand.getValueKind() == ValueKind::FLOATING || rightOperand.getValueKind() == ValueKind::FLOATING
            || result.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        Register& resultRegister = get64BitRegister();
        auto toXmm = [&](Value& v, const char* xmm) {
            Register& tmp = get64BitRegister();
            if (residesInMemory(v)) {
                emitLoad(v, tmp);
            } else {
                assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
            }
            if (v.getValueKind() == ValueKind::INTEGRAL) {
                assembly << (std::string("cvtsi2sd ") + xmm + ", " + tmp.getName());
            } else {
                assembly << (std::string("movq ") + xmm + ", " + tmp.getName());
            }
        };
        toXmm(leftOperand, "xmm0");
        toXmm(rightOperand, "xmm1");
        assembly << "divsd xmm0, xmm1";
        assembly << ("movq " + resultRegister.getName() + ", xmm0");
        bindResult(resultRegister, result);
        return;
    }

    if (result.getValueKind() != ValueKind::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    // Unsigned: zero-extend dividend (rdx:rax = 0:rax). Signed: sign-extend with cqo
    // so negative dividends do not become huge positive rdx:rax values (SIGFPE/garbage).
    if (unsignedDiv) {
        assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
        if (residesInMemory(rightOperand)) {
            assembly << instructionSet->div(memoryOperand(rightOperand));
        } else {
            assembly << instructionSet->div(rightOperand.getAssignedRegister());
        }
    } else {
        assembly << std::string("cqo");
        if (residesInMemory(rightOperand)) {
            assembly << instructionSet->idiv(memoryOperand(rightOperand));
        } else {
            assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
        }
    }
    bindResult(multiplicationRegister, result);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool unsignedMod) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    if (result.getValueKind() != ValueKind::INTEGRAL) {
        throw std::runtime_error{"modular division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    if (unsignedMod) {
        assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
        if (residesInMemory(rightOperand)) {
            assembly << instructionSet->div(memoryOperand(rightOperand));
        } else {
            assembly << instructionSet->div(rightOperand.getAssignedRegister());
        }
    } else {
        assembly << std::string("cqo");
        if (residesInMemory(rightOperand)) {
            assembly << instructionSet->idiv(memoryOperand(rightOperand));
        } else {
            assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
        }
    }
    bindResult(registers->getRemainderRegister(), result);
}

void StackMachine::inc(std::string operandName, int amount) {
    Value& operand = resolve(operandName);
    if (amount == 1) {
        if (residesInMemory(operand)) {
            assembly << instructionSet->inc(memoryOperand(operand));
        } else {
            assembly << instructionSet->inc(operand.getAssignedRegister());
        }
        return;
    }
    // Pointer ++ by sizeof(pointee): add amount (not x86 INC).
    if (residesInMemory(operand)) {
        Register& reg = assignRegisterTo(operand);
        assembly << instructionSet->add(reg, amount);
        emitStore(reg, operand);
    } else {
        assembly << instructionSet->add(operand.getAssignedRegister(), amount);
    }
}

void StackMachine::dec(std::string operandName, int amount) {
    Value& operand = resolve(operandName);
    if (amount == 1) {
        if (residesInMemory(operand)) {
            assembly << instructionSet->dec(memoryOperand(operand));
        } else {
            assembly << instructionSet->dec(operand.getAssignedRegister());
        }
        return;
    }
    if (residesInMemory(operand)) {
        Register& reg = assignRegisterTo(operand);
        assembly << instructionSet->sub(reg, amount);
        emitStore(reg, operand);
    } else {
        assembly << instructionSet->sub(operand.getAssignedRegister(), amount);
    }
}

void StackMachine::shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
        std::string (InstructionSet::*emitShift)(const Register&) const) {
    // Count must live in %cl (RCX) and be tracked so the value is not placed in RCX.
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = resolve(rightOperandName);
    if (residesInMemory(rightOperand)) {
        emitLoad(rightOperand, counterRegister);
    } else if (&counterRegister != &rightOperand.getAssignedRegister()) {
        assembly << instructionSet->mov(rightOperand.getAssignedRegister(), counterRegister);
        storeRegisterValue(rightOperand.getAssignedRegister());
    }
    // Count in %cl is scratch only; never register-cache a global home on RCX.
    if (!addressOf(rightOperand).isGlobal()) {
        counterRegister.assign(&rightOperand);
    }

    Value& leftOperand = resolve(leftOperandName);
    Register& resultRegister = get64BitRegisterExcluding(counterRegister);
    assignRegisterToSymbol(resultRegister, leftOperand);
    assembly << (instructionSet.get()->*emitShift)(resultRegister);
    Value& result = resolve(resultName);
    bindResult(resultRegister, result);
}

void StackMachine::shl(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shl);
}

void StackMachine::shr(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool logical) {
    shiftBy(leftOperandName, rightOperandName, resultName,
            logical ? &InstructionSet::shr : &InstructionSet::sar);
}

void StackMachine::bswap(std::string operandName, std::string resultName, int sizeBytes) {
    Value& operand = resolve(operandName);
    Register& resultRegister = get64BitRegister();
    if (residesInMemory(operand)) {
        emitLoad(operand, resultRegister);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
    }
    if (sizeBytes == 2) {
        // Swap the low 16 bits: rol ax, 8.
        assembly << ("rol " + reg16Name(resultRegister) + ", 8");
        assembly << ("movzx " + resultRegister.getName() + ", " + reg16Name(resultRegister));
    } else if (sizeBytes == 4) {
        assembly << ("bswap " + reg32Name(resultRegister));
        // Zero-extend so high bits do not leak into later 64-bit ops.
        assembly << ("mov " + reg32Name(resultRegister) + ", " + reg32Name(resultRegister));
    } else {
        assembly << ("bswap " + resultRegister.getName());
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::ctz(std::string operandName, std::string resultName) {
    // Count trailing zeros via BSF (matches GCC ctz on non-zero; 0 is UB in C).
    Value& operand = resolve(operandName);
    Register& resultRegister = get64BitRegister();
    if (residesInMemory(operand)) {
        emitLoad(operand, resultRegister);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
    }
    assembly << ("bsf " + resultRegister.getName() + ", " + resultRegister.getName());
    bindResult(resultRegister, resolve(resultName));
}


} // namespace codegen
