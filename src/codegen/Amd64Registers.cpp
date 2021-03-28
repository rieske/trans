#include "Amd64Registers.h"

namespace codegen {

Amd64Registers::~Amd64Registers() = default;

Register& codegen::Amd64Registers::getBasePointer() {
    return basePointer;
}

Register& Amd64Registers::getStackPointer() {
    return stackPointer;
}

std::vector<Register*> Amd64Registers::getGeneralPurposeRegisters() {
    return generalPurposeRegisters;
}

std::vector<Register*> Amd64Registers::getCallerSavedRegisters() {
    return callerSavedRegisters;
}

std::vector<Register*> Amd64Registers::getCalleeSavedRegisters() {
    return calleeSavedRegisters;
}

std::vector<Register*> Amd64Registers::getIntegerArgumentRegisters() {
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

} // namespace codegen


