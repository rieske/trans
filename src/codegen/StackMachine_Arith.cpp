#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace codegen {

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::mulss, &InstructionSet::mulsd);
        return;
    }

    if (result.getType() != Type::INTEGRAL) {
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

void StackMachine::emitIntegerDivide(Value& left, Value& right, bool signedDiv) {
    Register& rax = registers->getMultiplicationRegister();
    assignRegisterToSymbol(rax, left);
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(rdx);
    if (signedDiv) {
        // Sign-extend RAX into RDX:RAX (xor rdx,rdx breaks negatives).
        assembly << instructionSet->cqo();
    } else {
        assembly << instructionSet->xor_(rdx, rdx);
    }
    if (residesInMemory(right)) {
        assembly << (signedDiv ? instructionSet->idiv(memoryOperand(right))
                               : instructionSet->div(memoryOperand(right)));
    } else {
        assembly << (signedDiv ? instructionSet->idiv(right.getAssignedRegister())
                               : instructionSet->div(right.getAssignedRegister()));
    }
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName,
        bool signedDiv) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::divss, &InstructionSet::divsd);
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
