#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"
#include "types/ObjectAbi.h"

namespace codegen {

int StackMachine::promotedBytes(const Value& value) const {
    return value.getClassification().gprExtend == type::sysv::GprExtend::None ? 8 : 4;
}

void StackMachine::copyToRegister(Value& symbol, Register& dest) {
    if (residesInMemory(symbol)) {
        loadPromoted(symbol, dest);
        return;
    }
    Register& cur = symbol.getAssignedRegister();
    if (&cur != &dest) {
        assembly << instructionSet->mov(cur, dest);
    }
}

Register& StackMachine::materialize(Value& symbol) {
    if (residesInMemory(symbol)) {
        return assignRegisterTo(symbol);
    }
    return symbol.getAssignedRegister();
}

Register& StackMachine::materializeExcluding(Value& symbol, Register& exclude) {
    if (residesInMemory(symbol)) {
        return assignRegisterExcluding(symbol, exclude);
    }
    return symbol.getAssignedRegister();
}

void StackMachine::canonicalize(Register& reg, Value& symbol) {
    if (symbol.getClassification().gprExtend == type::sysv::GprExtend::None) {
        return;
    }
    storeObject(reg, symbol);
    loadPromoted(symbol, reg);
}

void StackMachine::emitGprBinary(Value& left, Value& right, Value& result, WideIntegerOp op) {
    Register& acc = get64BitRegister();
    copyToRegister(left, acc);
    const int width = promotedBytes(result);
    Register& rhs = materializeExcluding(right, acc);
    switch (op) {
    case WideIntegerOp::Add: assembly << instructionSet->add(rhs, acc, width); break;
    case WideIntegerOp::Sub: assembly << instructionSet->sub(rhs, acc, width); break;
    case WideIntegerOp::And: assembly << instructionSet->and_(rhs, acc, width); break;
    case WideIntegerOp::Or: assembly << instructionSet->or_(rhs, acc, width); break;
    case WideIntegerOp::Xor: assembly << instructionSet->xor_(rhs, acc, width); break;
    }
    bindResult(acc, result);
}

void StackMachine::mul(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::mulss, &InstructionSet::mulsd, X87Op::Mul);
        return;
    }

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"multiplication of non integers is not implemented"};
    }

    Register& rax = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(rax);
    storeRegisterValue(rdx);
    copyToRegister(leftOperand, rax);
    Register& rhs = get64BitRegisterExcluding(std::vector<Register*> { &rax, &rdx });
    copyToRegister(rightOperand, rhs);
    assembly << instructionSet->imul(rhs, promotedBytes(result));
    bindResult(rax, result);
}

void StackMachine::emitIntegerDivide(Value& left, Value& right, bool signedDiv) {
    Register& rax = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(rax);
    storeRegisterValue(rdx);
    copyToRegister(left, rax);
    Register& divisor = get64BitRegisterExcluding(std::vector<Register*> { &rax, &rdx });
    copyToRegister(right, divisor);
    const int width = promotedBytes(left);
    if (signedDiv) {
        assembly << (width == 4 ? instructionSet->cdq() : instructionSet->cqo());
    } else {
        assembly << instructionSet->xor_(rdx, rdx);
    }
    assembly << (signedDiv ? instructionSet->idiv(divisor, width)
                           : instructionSet->div(divisor, width));
}

void StackMachine::div(int leftOperandName, int rightOperandName, int resultName,
        bool signedDiv) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::divss, &InstructionSet::divsd, X87Op::Div);
        return;
    }

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    emitIntegerDivide(leftOperand, rightOperand, signedDiv);
    bindResult(registers->getMultiplicationRegister(), result);
}

void StackMachine::ctz(int operandName, int resultName, int widthBytes) {
    auto& operand = resolve(operandName);
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    assembly << instructionSet->ctz(resultRegister, widthBytes);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::mod(int leftOperandName, int rightOperandName, int resultName,
        bool signedDiv) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"modular division of non integer types is not implemented"};
    }

    emitIntegerDivide(leftOperand, rightOperand, signedDiv);
    bindResult(registers->getRemainderRegister(), result);
}

void StackMachine::compare(int leftSymbolName, int rightSymbolName, bool signedRel) {
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

    // Usual arithmetic: promote integral side to double bits before comparing.
    // IEEE bit patterns order as signed integers for non-NaN values.
    const bool floating = leftSymbol.getType() == Type::FLOATING
            || rightSymbol.getType() == Type::FLOATING;
    if (floating) {
        const bool destFloat32 = !isSseFloat64(leftSymbol) && !isSseFloat64(rightSymbol);
        loadValueToXmm(leftSymbol, 0, destFloat32);
        loadValueToXmm(rightSymbol, 1, destFloat32);
        Register& leftReg = get64BitRegister();
        Register& rightReg = get64BitRegisterExcluding(leftReg);
        xmmToGpr(0, leftReg, destFloat32);
        xmmToGpr(1, rightReg, destFloat32);
        assembly << instructionSet->cmp(leftReg, rightReg);
        return;
    }

    const int width = (promotedBytes(leftSymbol) == 4 && promotedBytes(rightSymbol) == 4) ? 4 : 8;
    Register& leftReg = materialize(leftSymbol);
    Register& rightReg = materializeExcluding(rightSymbol, leftReg);
    assembly << instructionSet->cmp(leftReg, rightReg, width);
}

void StackMachine::zeroCompare(int symbolName) {
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
    Register& reg = materialize(symbol);
    assembly << instructionSet->cmp(reg, 0, promotedBytes(symbol));
}
void StackMachine::unaryMinus(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    if (tryComplexUnaryMinus(operand, resolve(resultName))) {
        return;
    }
    if (isX87Float(operand)) {
        emitX87UnaryMinus(operand, resolve(resultName));
        return;
    }
    // IEEE float/double: flip sign bit (integer neg corrupts the bit pattern).
    if (operand.getType() == Type::FLOATING) {
        Register& resultRegister = residesInMemory(operand)
                ? get64BitRegister()
                : get64BitRegisterExcluding(operand.getAssignedRegister());
        if (residesInMemory(operand)) {
            emitLoad(operand, resultRegister);
        } else {
            assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
        }
        Register& mask = get64BitRegisterExcluding(resultRegister);
        const char* signBit = isSseFloat32(operand)
                ? "0x80000000" : "0x8000000000000000";
        assembly << instructionSet->mov(signBit, mask);
        assembly << instructionSet->xor_(mask, resultRegister);
        bindResult(resultRegister, resolve(resultName));
        return;
    }
    Value& result = resolve(resultName);
    if (tryWideUnaryMinus(operand, result)) {
        return;
    }
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    assembly << instructionSet->neg(resultRegister, promotedBytes(result));
    bindResult(resultRegister, result);
}

void StackMachine::bswap(int operandName, int resultName, int widthBytes) {
    auto& operand = resolve(operandName);
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    for (const auto& insn : instructionSet->bswap(resultRegister, widthBytes)) {
        assembly << insn;
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::unaryNot(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    Value& result = resolve(resultName);
    if (tryWideUnaryNot(operand, result)) {
        return;
    }
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    assembly << instructionSet->not_(resultRegister, promotedBytes(result));
    bindResult(resultRegister, result);
}

void StackMachine::widenInteger(int operandName, int resultName, bool signHighWord) {
    Value& operand = resolve(operandName);
    Value& result = resolve(resultName);
    storeInMemory(operand);
    if (type::object_abi::valueWords(result.getSizeInBytes()) <= 1) {
        Register& dest = get64BitRegister();
        loadWord(operand, 0, dest);
        bindResult(dest, result);
        return;
    }
    Register& lo = registers->getRetrievalRegister();
    Register& hi = registers->getRemainderRegister();
    storeRegisterValue(lo);
    storeRegisterValue(hi);
    loadWord(operand, 0, lo);
    if (signHighWord) {
        assembly << instructionSet->cqo();
    } else {
        assembly << instructionSet->xor_(hi, hi);
    }
    storeWord(lo, result, 0);
    storeWord(hi, result, 1);
}

void StackMachine::assign(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    if (tryComplexAssignConvert(operand, result)) {
        return;
    }
    if (tryNumericAssignConvert(operand, result)) {
        return;
    }

    if (type::object_abi::valueWords(operand.getSizeInBytes()) > 1
            || type::object_abi::valueWords(result.getSizeInBytes()) > 1) {
        copyWords(operand, result);
        return;
    }

    if (residesInMemory(operand) && residesInMemory(result)) {
        Register& reg = get64BitRegister();
        emitLoad(operand, reg);
        emitStore(reg, result);
    } else if (residesInMemory(operand)) {
        emitLoad(operand, result.getAssignedRegister());
    } else if (residesInMemory(result)) {
        emitStore(operand.getAssignedRegister(), result);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
    if (!residesInMemory(result)) {
        bindResult(result.getAssignedRegister(), result);
    }
}

void StackMachine::assignConstant(int constant, int resultName, int highWord) {
    auto& result = resolve(resultName);
    if (type::object_abi::valueWords(result.getSizeInBytes()) > 1) {
        Register& lo = get64BitRegister();
        assembly << instructionSet->mov(text(constant), lo);
        storeWord(lo, result, 0);
        Register& hi = get64BitRegisterExcluding(lo);
        assembly << instructionSet->mov(highWord < 0 ? "0" : text(highWord), hi);
        storeWord(hi, result, 1);
        return;
    }
    // Float IEEE bits and large integers exceed signed 32-bit imm to memory; go via register.
    Register& reg = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    assembly << instructionSet->mov(text(constant), reg);
    if (residesInMemory(result)) {
        emitStore(reg, result);
    } else {
        bindResult(reg, result);
    }
}

void StackMachine::assignLabelAddress(int label, int resultName) {
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(MemoryOperand::global(text(label)), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::lvalueAssign(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    // Anything cached in a register is stale after this store; write it back first.
    spillGeneralPurposeRegisters();

    const int storeSize = operand.getSizeInBytes();
    if (!nativeMoveSize(storeSize)) {
        Register& ptr = residesInMemory(result) ? assignRegisterTo(result) : result.getAssignedRegister();
        copyToPointer(operand, ptr);
        return;
    }

    Register& operandRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = residesInMemory(result) ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    storeObject(operandRegister, MemoryOperand::at(resultRegister, 0), storeSize);
}

void StackMachine::xorCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Xor)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Xor);
}

void StackMachine::orCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Or)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Or);
}

void StackMachine::andCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::And)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::And);
}

bool StackMachine::involvesFloating(const Value& left, const Value& right, const Value& result) const {
    return left.getType() == Type::FLOATING || right.getType() == Type::FLOATING
            || result.getType() == Type::FLOATING;
}

void StackMachine::add(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryComplexBinary(leftOperand, rightOperand, result, X87Op::Add)) {
        return;
    }
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::addss, &InstructionSet::addsd, X87Op::Add);
        return;
    }
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Add)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Add);
}

void StackMachine::sub(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryComplexBinary(leftOperand, rightOperand, result, X87Op::Sub)) {
        return;
    }
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::subss, &InstructionSet::subsd, X87Op::Sub);
        return;
    }
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub);
}

void StackMachine::stepLvalue(Value& operand, bool increment, int step) {
    Register& reg = get64BitRegister();
    copyToRegister(operand, reg);
    if (!residesInMemory(operand)) {
        operand.getAssignedRegister().free();
    }
    if (step == 1) {
        assembly << (increment ? instructionSet->inc(reg, promotedBytes(operand))
                               : instructionSet->dec(reg, promotedBytes(operand)));
    } else if (increment) {
        assembly << instructionSet->add(reg, step);
    } else {
        assembly << instructionSet->sub(reg, step);
    }
    bindResult(reg, operand);
}

void StackMachine::inc(int operandName, int step) {
    stepLvalue(resolve(operandName), true, step);
}

void StackMachine::dec(int operandName, int step) {
    stepLvalue(resolve(operandName), false, step);
}

void StackMachine::shiftBy(int leftOperandName, int rightOperandName, int resultName,
        std::string (InstructionSet::*emitShift)(const Register&, int) const) {
    // Count must live in %cl (RCX) and be tracked so the value is not placed in RCX.
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = resolve(rightOperandName);
    copyToRegister(rightOperand, counterRegister);
    if (!addressOf(rightOperand).isGlobal()) {
        if (!residesInMemory(rightOperand)
                && &rightOperand.getAssignedRegister() != &counterRegister) {
            rightOperand.getAssignedRegister().free();
        }
        counterRegister.assign(&rightOperand);
    }

    Value& leftOperand = resolve(leftOperandName);
    Register& resultRegister = get64BitRegisterExcluding(counterRegister);
    copyToRegister(leftOperand, resultRegister);
    assembly << (instructionSet->*emitShift)(resultRegister, promotedBytes(leftOperand));
    Value& result = resolve(resultName);
    bindResult(resultRegister, result);
}

void StackMachine::shl(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideShift(leftOperand, rightOperand, result, WideShiftOp::Left)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shl);
}

void StackMachine::shr(int leftOperandName, int rightOperandName, int resultName,
        bool arithmetic) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    const WideShiftOp op = arithmetic ? WideShiftOp::ArithmeticRight : WideShiftOp::LogicalRight;
    if (tryWideShift(leftOperand, rightOperand, result, op)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName,
            arithmetic ? &InstructionSet::shr : &InstructionSet::lshr);
}
} // namespace codegen
