#include "StackMachine.h"

#include "types/ObjectAbi.h"

#include <stdexcept>

namespace codegen {

void StackMachine::scaleIntegerIntoRax(Value& index, int elementSizeBytes) {
    // One-operand imul writes RDX:RAX; free both first.
    Register& mulReg = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    storeInMemory(index);
    storeRegisterValue(mulReg);
    storeRegisterValue(rdx);
    loadWord(index, 0, mulReg);
    if (elementSizeBytes != 1) {
        Register& scaleReg = get64BitRegisterExcluding(mulReg);
        assembly << instructionSet->mov(std::to_string(elementSizeBytes), scaleReg);
        assembly << instructionSet->imul(scaleReg);
    }
}

void StackMachine::materializeBaseAddress(Value& base, symbols::AddressBaseMode baseMode, Register& dest) {
    if (symbols::addressBaseUsesLea(baseMode)) {
        storeInMemory(base);
        assembly << instructionSet->lea(memoryOperand(base), dest);
    } else if (residesInMemory(base)) {
        emitLoad(base, dest);
    } else {
        assembly << instructionSet->mov(base.getAssignedRegister(), dest);
    }
}

void StackMachine::scaledBaseIndex(int baseName, int indexName, int elementSizeBytes, int resultName,
        symbols::AddressBaseMode baseMode, bool subtract) {
    auto& base = resolve(baseName);
    auto& index = resolve(indexName);
    Register& mulReg = registers->getMultiplicationRegister();

    scaleIntegerIntoRax(index, elementSizeBytes);

    Register& addr = get64BitRegisterExcluding(mulReg);
    materializeBaseAddress(base, baseMode, addr);
    if (subtract) {
        assembly << instructionSet->sub(mulReg, addr);
    } else {
        assembly << instructionSet->add(mulReg, addr);
    }
    bindResult(addr, resolve(resultName));
}

void StackMachine::indexAddress(int baseName, int indexName, int elementSizeBytes, int resultName,
        symbols::AddressBaseMode baseMode) {
    scaledBaseIndex(baseName, indexName, elementSizeBytes, resultName, baseMode, false);
}

void StackMachine::pointerOffset(int baseName, int indexName, int elementSizeBytes, int resultName,
        bool subtract) {
    scaledBaseIndex(baseName, indexName, elementSizeBytes, resultName, symbols::AddressBaseMode::PointerValue, subtract);
}

void StackMachine::pointerDifference(int leftName, int rightName, int elementSizeBytes,
        int resultName) {
    auto& left = resolve(leftName);
    auto& right = resolve(rightName);
    Register& mulReg = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();

    Register& addr = get64BitRegisterExcluding(mulReg);
    copyToRegister(left, addr);
    Register& rhs = materializeExcluding(right, addr);
    assembly << instructionSet->sub(rhs, addr);

    if (elementSizeBytes == 1) {
        bindResult(addr, resolve(resultName));
        return;
    }

    // Signed element count: byte_diff / stride. idiv uses RDX:RAX; divisor not RAX/RDX.
    storeRegisterValue(mulReg);
    storeRegisterValue(rdx);
    assembly << instructionSet->mov(addr, mulReg);
    assembly << instructionSet->cqo();
    Register* divisor = nullptr;
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &mulReg && reg != &rdx) {
            storeRegisterValue(*reg);
            divisor = reg;
            break;
        }
    }
    if (!divisor) {
        throw std::runtime_error { "no free register for pointer difference divisor" };
    }
    assembly << instructionSet->mov(std::to_string(elementSizeBytes), *divisor);
    assembly << instructionSet->idiv(*divisor);
    assembly << instructionSet->mov(mulReg, addr);
    bindResult(addr, resolve(resultName));
}

void StackMachine::fieldAddress(int baseName, int offsetBytes, int resultName,
        symbols::AddressBaseMode baseMode) {
    auto& base = resolve(baseName);
    Register& addr = get64BitRegister();
    materializeBaseAddress(base, baseMode, addr);
    if (offsetBytes != 0) {
        assembly << instructionSet->add(addr, offsetBytes);
    }
    bindResult(addr, resolve(resultName));
}

void StackMachine::allocaBytes(int sizeName, int resultName) {
    auto& size = resolve(sizeName);
    Register& sizeReg = residesInMemory(size)
            ? get64BitRegister()
            : get64BitRegisterExcluding(size.getAssignedRegister());
    if (residesInMemory(size)) {
        emitLoad(size, sizeReg);
    } else {
        assembly << instructionSet->mov(size.getAssignedRegister(), sizeReg);
    }
    const int alignment = type::object_abi::STACK_ALIGNMENT;
    assembly << instructionSet->add(sizeReg, alignment - 1);
    Register& mask = get64BitRegisterExcluding(sizeReg);
    assembly << instructionSet->mov(std::to_string(-alignment), mask);
    assembly << instructionSet->and_(mask, sizeReg);
    assembly << instructionSet->sub(sizeReg, registers->getStackPointer());
    Register& resultRegister = get64BitRegisterExcluding(sizeReg);
    assembly << instructionSet->mov(registers->getStackPointer(), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

} // namespace codegen
