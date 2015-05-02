#ifndef AMD64REGISTERS_H_
#define AMD64REGISTERS_H_

#include <vector>

#include "Register.h"

namespace codegen {

class Amd64Registers {
public:
    virtual ~Amd64Registers() = default;

    Register& getBasePointer();
    Register& getStackPointer();
    std::vector<Register*> getGeneralPurposeRegisters();
    std::vector<Register*> getIntegerArgumentRegisters();
    Register& getRetrievalRegister();
    Register& getMultiplicationRegister();
    Register& getRemainderRegister();

private:
    Register rax { "rax" };
    Register rbx { "rbx" };
    Register rcx { "rcx" };
    Register rdx { "rdx" };
    Register rsi { "rsi" };
    Register rdi { "rdi" };

    Register r8 { "r8" };
    Register r9 { "r9" };
    Register r10 { "r10" };
    Register r11 { "r11" };
    Register r12 { "r12" };
    Register r13 { "r13" };
    Register r14 { "r14" };
    Register r15 { "r15" };

    Register stackPointer { "rsp" };
    Register basePointer { "rbp" };

    std::vector<Register*> generalPurposeRegisters { &rax, &rbx, &rcx, &rdx, &rdi, &rsi, &r8, &r9, &r10, &r11, &r12, &r13, &r14, &r15 };
    std::vector<Register*> integerArgumentRegisters { &rdi, &rsi, &rdx, &rcx, &r8, &r9 };
};

} /* namespace codegen */

#endif /* AMD64REGISTERS_H_ */
