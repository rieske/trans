#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "types/ObjectAbi.h"

namespace codegen {

GprWidth StackMachine::aluWidth(const Value& op) const {
    return (op.getValueKind() == ValueKind::INTEGRAL && op.getSizeInBytes() == 4)
            ? GprWidth::W32 : GprWidth::W64;
}

void StackMachine::signExtendIfNarrow(Register& reg, const Value& v, GprWidth width) {
    if (width == GprWidth::W32 && v.isSigned()) {
        assembly << instructionSet->extend(reg, 4, true);
    }
}

void StackMachine::commitIntegral(Register& reg, Value& result, GprWidth width) {
    signExtendIfNarrow(reg, result, width);
    bindResult(reg, result);
}

Register& StackMachine::copyToNewRegister(Value& src) {
    Register& dest = residesInMemory(src)
            ? get64BitRegister()
            : get64BitRegisterExcluding(src.getAssignedRegister());
    if (residesInMemory(src)) {
        emitLoad(src, dest);
    } else {
        assembly << instructionSet->mov(src.getAssignedRegister(), dest);
    }
    return dest;
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

    const GprWidth width = aluWidth(leftSymbol);
    Register& leftReg = residesInMemory(leftSymbol)
            ? assignRegisterTo(leftSymbol)
            : leftSymbol.getAssignedRegister();
    Register& rightReg = residesInMemory(rightSymbol)
            ? assignRegisterExcluding(rightSymbol, leftReg)
            : rightSymbol.getAssignedRegister();
    assembly << instructionSet->cmp(leftReg, rightReg, width);
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
    const GprWidth width = aluWidth(symbol);
    Register& reg = residesInMemory(symbol)
            ? assignRegisterTo(symbol)
            : symbol.getAssignedRegister();
    assembly << instructionSet->cmp(reg, 0, width);
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
        Register& resultRegister = copyToNewRegister(operand);
        Register& mask = get64BitRegisterExcluding(resultRegister);
        const char* signBit = sseWidth(operand) == SseWidth::F32
                ? "0x80000000" : "0x8000000000000000";
        assembly << instructionSet->mov(signBit, mask);
        // xor_(operand, result) => result ^= operand
        assembly << instructionSet->xor_(mask, resultRegister, GprWidth::W64);
        bindResult(resultRegister, resolve(resultName));
        return;
    }
    Value& result = resolve(resultName);
    if (tryWideUnaryMinus(operand, result)) {
        return;
    }
    const GprWidth width = aluWidth(result);
    Register& resultRegister = copyToNewRegister(operand);
    assembly << instructionSet->neg(resultRegister, width);
    commitIntegral(resultRegister, result, width);
}

void StackMachine::bitwiseNot(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    Value& result = resolve(resultName);
    if (tryWideUnaryNot(operand, result)) {
        return;
    }
    const GprWidth width = aluWidth(result);
    Register& resultRegister = copyToNewRegister(operand);
    assembly << instructionSet->not_(resultRegister, width);
    commitIntegral(resultRegister, result, width);
}

void StackMachine::emitIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp wide,
        std::string (InstructionSet::*regOp)(const Register&, const Register&, GprWidth) const) {
    if (tryWideIntegerBinary(left, right, result, wide)) {
        return;
    }
    Register& resultRegister = copyToNewRegister(left);
    const GprWidth width = aluWidth(result);
    if (residesInMemory(right)) {
        Register& rightReg = get64BitRegisterExcluding(resultRegister);
        emitLoad(right, rightReg);
        assembly << ((*instructionSet).*regOp)(rightReg, resultRegister, width);
    } else {
        assembly << ((*instructionSet).*regOp)(right.getAssignedRegister(), resultRegister, width);
    }
    commitIntegral(resultRegister, result, width);
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::Xor, &InstructionSet::xor_);
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::Or, &InstructionSet::or_);
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    emitIntegerBinary(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            WideIntegerOp::And, &InstructionSet::and_);
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (usesFpPath(leftOperand) || usesFpPath(rightOperand) || usesFpPath(result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Add, X87Op::Add);
        return;
    }
    emitIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Add, &InstructionSet::add);
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (usesFpPath(leftOperand) || usesFpPath(rightOperand) || usesFpPath(result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result, SseBin::Sub, X87Op::Sub);
        return;
    }
    emitIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub, &InstructionSet::sub);
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
    const GprWidth width = aluWidth(result);
    if (residesInMemory(rightOperand)) {
        Register& rightReg = get64BitRegisterExcluding(multiplicationRegister);
        emitLoad(rightOperand, rightReg);
        assembly << instructionSet->imul(rightReg, width);
    } else {
        assembly << instructionSet->imul(rightOperand.getAssignedRegister(), width);
    }
    commitIntegral(multiplicationRegister, result, width);
}

void StackMachine::emitIntegerDiv(Value& left, Value& right, Value& result, bool unsignedOp, bool remainder) {
    if (result.getValueKind() != ValueKind::INTEGRAL) {
        throw std::runtime_error{remainder
                ? "modular division of non integer types is not implemented"
                : "division of non integer types is not implemented"};
    }

    Register& quotient = registers->getMultiplicationRegister();
    assignRegisterToSymbol(quotient, left);
    storeRegisterValue(registers->getRemainderRegister());
    const GprWidth width = aluWidth(result);
    Register* divisor = nullptr;
    if (residesInMemory(right)) {
        divisor = &get64BitRegisterExcluding(
                { &quotient, &registers->getRemainderRegister() });
        emitLoad(right, *divisor);
    } else {
        divisor = &right.getAssignedRegister();
    }
    if (unsignedOp) {
        assembly << instructionSet->xor_(registers->getRemainderRegister(),
                registers->getRemainderRegister(), GprWidth::W64);
        assembly << instructionSet->div(*divisor, width);
    } else {
        assembly << (width == GprWidth::W32 ? instructionSet->cdq() : instructionSet->cqo());
        assembly << instructionSet->idiv(*divisor, width);
    }
    Register& out = remainder ? registers->getRemainderRegister() : quotient;
    commitIntegral(out, result, width);
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
    emitIntegerDiv(leftOperand, rightOperand, result, unsignedDiv, false);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName, bool unsignedMod) {
    emitIntegerDiv(resolve(leftOperandName), resolve(rightOperandName), resolve(resultName),
            unsignedMod, true);
}

void StackMachine::adjust(std::string operandName, int amount, bool plus) {
    Value& operand = resolve(operandName);
    const GprWidth width = aluWidth(operand);
    if (amount == 1) {
        // Materialize so a 1- or 2-byte home is never incq/addq. Store the
        // truncated object; do not keep the ALU register as the live value.
        Register& reg = get64BitRegister();
        if (residesInMemory(operand)) {
            emitLoad(operand, reg);
        } else {
            Register& cur = operand.getAssignedRegister();
            if (&cur != &reg) {
                assembly << instructionSet->mov(cur, reg);
            }
            cur.free();
        }
        if (plus) {
            assembly << instructionSet->inc(reg, width);
        } else {
            assembly << instructionSet->dec(reg, width);
        }
        emitStore(reg, operand);
        return;
    }
    if (residesInMemory(operand)) {
        Register& reg = assignRegisterTo(operand);
        assembly << (plus
                ? instructionSet->add(reg, amount, width)
                : instructionSet->sub(reg, amount, width));
        emitStore(reg, operand);
    } else {
        assembly << (plus
                ? instructionSet->add(operand.getAssignedRegister(), amount, width)
                : instructionSet->sub(operand.getAssignedRegister(), amount, width));
        signExtendIfNarrow(operand.getAssignedRegister(), operand, width);
    }
}

void StackMachine::inc(std::string operandName, int amount) {
    adjust(operandName, amount, true);
}

void StackMachine::dec(std::string operandName, int amount) {
    adjust(operandName, amount, false);
}

void StackMachine::shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
        std::string (InstructionSet::*emitShift)(const Register&, GprWidth) const) {
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
    Value& result = resolve(resultName);
    const GprWidth width = aluWidth(leftOperand);
    assembly << (instructionSet.get()->*emitShift)(resultRegister, width);
    commitIntegral(resultRegister, result, width);
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
    Register& resultRegister = copyToNewRegister(operand);
    for (const auto& insn : instructionSet->bswap(resultRegister, sizeBytes)) {
        assembly << insn;
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::ctz(std::string operandName, std::string resultName, int widthBytes) {
    Value& operand = resolve(operandName);
    Register& resultRegister = copyToNewRegister(operand);
    assembly << instructionSet->ctz(resultRegister, widthBytes);
    bindResult(resultRegister, resolve(resultName));
}


} // namespace codegen
