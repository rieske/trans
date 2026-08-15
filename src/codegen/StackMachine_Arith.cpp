#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace codegen {

int StackMachine::integerWidth(const Value& value) const {
    return (value.getType() == Type::INTEGRAL && value.getSizeInBytes() == 4) ? 4 : 8;
}

bool StackMachine::extendsFromMemory(const Value& value) const {
    return value.getType() == Type::INTEGRAL
            && value.getClassification().gprExtend != type::sysv::GprExtend::None;
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

void StackMachine::emitUnitStep(Value& operand, bool increment) {
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
    if (increment) {
        assembly << instructionSet->inc(reg, integerWidth(operand));
    } else {
        assembly << instructionSet->dec(reg, integerWidth(operand));
    }
    emitStore(reg, operand);
}

void StackMachine::emitGprBinary(Value& left, Value& right, Value& result, WideIntegerOp op) {
    Register& acc = get64BitRegister();
    if (residesInMemory(left)) {
        emitLoad(left, acc);
    } else {
        assembly << instructionSet->mov(left.getAssignedRegister(), acc);
    }
    const int width = integerWidth(result);
    Register& rightReg = materializeExcluding(right, acc);
    switch (op) {
    case WideIntegerOp::Add: assembly << instructionSet->add(rightReg, acc, width); break;
    case WideIntegerOp::Sub: assembly << instructionSet->sub(rightReg, acc, width); break;
    case WideIntegerOp::And: assembly << instructionSet->and_(rightReg, acc, width); break;
    case WideIntegerOp::Or: assembly << instructionSet->or_(rightReg, acc, width); break;
    case WideIntegerOp::Xor: assembly << instructionSet->xor_(rightReg, acc, width); break;
    }
    bindResult(acc, result);
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
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

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    // imul writes RDX:RAX; spill RDX if it holds a live value (e.g. pointer for *p *= ...)
    storeRegisterValue(registers->getRemainderRegister());
    const int width = integerWidth(result);
    assembly << instructionSet->imul(materializeExcluding(rightOperand, multiplicationRegister), width);
    bindResult(multiplicationRegister, result);
}

void StackMachine::emitIntegerDivide(Value& left, Value& right, bool signedDiv) {
    Register& rax = registers->getMultiplicationRegister();
    assignRegisterToSymbol(rax, left);
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(rdx);
    const int width = integerWidth(left);
    if (signedDiv) {
        assembly << (width == 4 ? instructionSet->cdq() : instructionSet->cqo());
    } else {
        assembly << instructionSet->xor_(rdx, rdx);
    }
    if (residesInMemory(right)) {
        Register& loaded = get64BitRegisterExcluding(std::vector<Register*> { &rax, &rdx });
        emitLoad(right, loaded);
        assembly << (signedDiv ? instructionSet->idiv(loaded, width)
                               : instructionSet->div(loaded, width));
    } else {
        assembly << (signedDiv ? instructionSet->idiv(right.getAssignedRegister(), width)
                               : instructionSet->div(right.getAssignedRegister(), width));
    }
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName,
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

void StackMachine::ctz(std::string operandName, std::string resultName, int widthBytes) {
    auto& operand = resolve(operandName);
    Register& resultRegister = residesInMemory(operand)
            ? get64BitRegister()
            : get64BitRegisterExcluding(operand.getAssignedRegister());
    if (residesInMemory(operand)) {
        emitLoad(operand, resultRegister);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
    }
    assembly << instructionSet->ctz(resultRegister, widthBytes);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName,
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
