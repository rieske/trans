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

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName, bool signedRel) {
    auto& leftSymbol = resolve(leftSymbolName);
    auto& rightSymbol = resolve(rightSymbolName);
    if (tryComplexCompare(leftSymbol, rightSymbol)) {
        return;
    }
    if (isX87Float(leftSymbol) || isX87Float(rightSymbol)) {
        emitX87Compare(leftSymbol, rightSymbol, signedRel);
        return;
    }
    if (tryWideCompare(leftSymbol, rightSymbol, signedRel)) {
        return;
    }

    // Floating relational: promote integrals to xmm, then ucomi (CF/ZF), not
    // a signed GPR bit-compare (two negatives reverse under two's complement).
    const bool floating = leftSymbol.getValueKind() == ValueKind::FLOATING
            || rightSymbol.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        const SseWidth destWidth = (isSseFloat64(leftSymbol) || isSseFloat64(rightSymbol)
                || isX87Float(leftSymbol) || isX87Float(rightSymbol))
                ? SseWidth::F64 : SseWidth::F32;
        loadValueToXmm(leftSymbol, 0, destWidth);
        loadValueToXmm(rightSymbol, 1, destWidth);
        assembly << instructionSet->sseUcomi(destWidth, 0, 1);
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
    if (tryComplexZeroCompare(symbol)) {
        return;
    }
    if (isX87Float(symbol)) {
        emitX87ZeroCompare(symbol);
        return;
    }
    if (tryWideZeroCompare(symbol)) {
        return;
    }
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
    if (tryComplexUnaryMinus(operand, resolve(resultName))) {
        return;
    }
    if (isX87Float(operand)) {
        emitX87UnaryMinus(operand, resolve(resultName));
        return;
    }
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
        const char* signBit = sseWidth(operand) == SseWidth::F32
                ? "0x80000000" : "0x8000000000000000";
        assembly << instructionSet->mov(signBit, mask);
        // xor_(operand, result) => result ^= operand
        assembly << instructionSet->xor_(mask, resultRegister);
        bindResult(resultRegister, resolve(resultName));
        return;
    }
    Value& result = resolve(resultName);
    if (tryWideUnaryMinus(operand, result)) {
        return;
    }
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, result);
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, result);
    }
}

void StackMachine::bitwiseNot(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    Value& result = resolve(resultName);
    if (tryWideUnaryNot(operand, result)) {
        return;
    }
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, result);
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, result);
    }
}

void StackMachine::emitIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp wide,
        std::string (InstructionSet::*memOp)(const MemoryOperand&, const Register&) const,
        std::string (InstructionSet::*regOp)(const Register&, const Register&) const) {
    if (tryWideIntegerBinary(left, right, result, wide)) {
        return;
    }
    Register& resultRegister = get64BitRegister();
    if (residesInMemory(left)) {
        emitLoad(left, resultRegister);
    } else {
        assembly << instructionSet->mov(left.getAssignedRegister(), resultRegister);
    }
    // Sub-word stack slots must not be used as qword memory operands: a store
    // through T* only writes the type width. Load+narrow first.
    const bool subwordRight = right.getValueKind() == ValueKind::INTEGRAL
            && right.getSizeInBytes() > 0 && right.getSizeInBytes() < MACHINE_WORD_SIZE;
    if (residesInMemory(right)) {
        if (subwordRight) {
            Register& rightReg = get64BitRegisterExcluding(resultRegister);
            emitLoad(right, rightReg);
            assembly << ((*instructionSet).*regOp)(rightReg, resultRegister);
        } else {
            assembly << ((*instructionSet).*memOp)(memoryOperand(right), resultRegister);
        }
    } else {
        assembly << ((*instructionSet).*regOp)(right.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, result);
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::Xor, &InstructionSet::xor_, &InstructionSet::xor_);
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::Or, &InstructionSet::or_, &InstructionSet::or_);
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::And, &InstructionSet::and_, &InstructionSet::and_);
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (usesFpPath(leftOperand) || usesFpPath(rightOperand) || usesFpPath(result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Add, X87Op::Add);
        return;
    }
    emitIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Add,
            &InstructionSet::add, &InstructionSet::add);
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (usesFpPath(leftOperand) || usesFpPath(rightOperand) || usesFpPath(result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Sub, X87Op::Sub);
        return;
    }
    emitIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub,
            &InstructionSet::sub, &InstructionSet::sub);
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    const bool floating = leftOperand.getValueKind() == ValueKind::FLOATING || rightOperand.getValueKind() == ValueKind::FLOATING
            || result.getValueKind() == ValueKind::FLOATING;
    if (floating) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Mul, X87Op::Mul);
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
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Div, X87Op::Div);
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
        assembly << instructionSet->cqo();
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
        assembly << instructionSet->cqo();
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
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideShift(leftOperand, rightOperand, result, WideShiftOp::Left)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shl);
}

void StackMachine::shr(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool logical) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    const WideShiftOp op = logical ? WideShiftOp::LogicalRight : WideShiftOp::ArithmeticRight;
    if (tryWideShift(leftOperand, rightOperand, result, op)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName,
            logical ? &InstructionSet::shr : &InstructionSet::sar);
}

void StackMachine::bswap(std::string operandName, std::string resultName, int sizeBytes) {
    Value& operand = resolve(operandName);
    Register& resultRegister = residesInMemory(operand)
            ? get64BitRegister()
            : get64BitRegisterExcluding(operand.getAssignedRegister());
    if (residesInMemory(operand)) {
        emitLoad(operand, resultRegister);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
    }
    for (const auto& insn : instructionSet->bswap(resultRegister, sizeBytes)) {
        assembly << insn;
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
    assembly << instructionSet->bsf(resultRegister);
    bindResult(resultRegister, resolve(resultName));
}


} // namespace codegen
