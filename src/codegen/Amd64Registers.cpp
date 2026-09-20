#include "Amd64Registers.h"

namespace codegen {

Register& codegen::Amd64Registers::getBasePointer() {
    return basePointer;
}

Register& Amd64Registers::getStackPointer() {
    return stackPointer;
}

const std::vector<Register*>& Amd64Registers::getGeneralPurposeRegisters() const {
    return generalPurposeRegisters;
}

const std::vector<Register*>& Amd64Registers::getCallerSavedRegisters() const {
    return callerSavedRegisters;
}

const std::vector<Register*>& Amd64Registers::getCalleeSavedRegisters() const {
    return calleeSavedRegisters;
}

const std::vector<Register*>& Amd64Registers::getIntegerArgumentRegisters() const {
    return integerArgumentRegisters;
}

Register& Amd64Registers::getRetrievalRegister() {
    return rax;
}

Register& Amd64Registers::getMultiplicationRegister() {
    return rax;
}

Register& Amd64Registers::getRemainderRegister() {
    return rdx;
}

Register& Amd64Registers::getCounterRegister() {
    return rcx;
}

Register& Amd64Registers::getIndirectCallTargetRegister() {
    return r10;
}

} // namespace codegen
