#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace codegen {

Register& StackMachine::loadIntegerAluOperand(Value& value, bool signedExt,
        const std::vector<Register*>& exclude) {
    Register* dest = nullptr;
    if (!residesInMemory(value)) {
        Register& held = value.getAssignedRegister();
        bool clash = false;
        for (Register* banned : exclude) {
            if (banned == &held) {
                clash = true;
                break;
            }
        }
        if (!clash) {
            dest = &held;
        }
    }
    if (dest == nullptr) {
        dest = &get64BitRegisterExcluding(exclude);
        if (residesInMemory(value)) {
            assembly << instructionSet->mov(memoryOperand(value), *dest);
        } else {
            assembly << instructionSet->mov(value.getAssignedRegister(), *dest);
        }
    }
    if (value.getSizeInBytes() > 0 && value.getSizeInBytes() < 8) {
        assembly << instructionSet->extendRegister(*dest, value.getSizeInBytes(), signedExt);
    }
    return *dest;
}

Register& StackMachine::loadIntegerAluOperand(Value& value, bool signedExt, Register& dest) {
    assignRegisterToSymbol(dest, value);
    if (value.getSizeInBytes() > 0 && value.getSizeInBytes() < 8) {
        assembly << instructionSet->extendRegister(dest, value.getSizeInBytes(), signedExt);
    }
    return dest;
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
    Register& rdx = registers->getRemainderRegister();
    // imul writes RDX:RAX; spill RDX if it holds a live value (e.g. pointer for *p *= ...)
    storeRegisterValue(rdx);
    loadIntegerAluOperand(leftOperand, false, multiplicationRegister);
    Register& rightReg = loadIntegerAluOperand(rightOperand, false, { &multiplicationRegister, &rdx });
    assembly << instructionSet->imul(rightReg);
    bindResult(multiplicationRegister, result);
}

void StackMachine::emitIntegerDivide(Value& left, Value& right, bool signedDiv) {
    Register& rax = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    loadIntegerAluOperand(left, signedDiv, rax);
    Register& divisor = loadIntegerAluOperand(right, signedDiv, { &rax, &rdx });
    storeRegisterValue(rdx);
    if (signedDiv) {
        assembly << instructionSet->cqo();
    } else {
        assembly << instructionSet->xor_(rdx, rdx);
    }
    assembly << (signedDiv ? instructionSet->idiv(divisor) : instructionSet->div(divisor));
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
