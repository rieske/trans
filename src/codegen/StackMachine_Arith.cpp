#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

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

} // namespace codegen
