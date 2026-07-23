#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "ObjectAbi.h"

namespace codegen {

void StackMachine::vaStart(std::string apName, std::string lastStorageName) {
    // Fill SysV va_list tag: apName is array-of-1 object or pointer-to-tag;
    // lastStorageName empty => C23 form; else lea of that storage for &last.
    // Do not push on the stack: frame homes are often RSP-relative, and a push
    // would shift them underfoot.
    static int seq = 0;
    const int id = ++seq;
    const std::string L_not_in_gp = ".Lva_not_gp_" + std::to_string(id);
    const std::string L_done = ".Lva_done_" + std::to_string(id);

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    // r12 = reg_save, r13 = overflow (callee-saved; prologue already preserves them).
    assembly << instructionSet->call("__trans_va_get_reg_save");
    assembly << std::string("mov r12, rax");
    assembly << instructionSet->call("__trans_va_get_overflow");
    assembly << std::string("mov r13, rax");

    // r8 = pointer to tag
    auto& ap = resolve(apName);
    storeInMemory(ap);
    Register& scratch = registers->getRetrievalRegister();
    if (ap.getSizeInBytes() > MACHINE_WORD_SIZE) {
        assembly << instructionSet->lea(memoryOperand(ap), scratch);
    } else {
        emitLoad(ap, scratch);
    }
    assembly << ("mov r8, " + scratch.getName());

    // fp_offset = 48
    assembly << std::string("mov dword [r8 + 4], 48");
    // reg_save_area = save
    assembly << std::string("mov [r8 + 16], r12");

    if (lastStorageName.empty()) {
        assembly << std::string("mov dword [r8], 0");
        assembly << std::string("mov [r8 + 8], r13");
    } else {
        // r9 = &last
        auto& last = resolve(lastStorageName);
        storeInMemory(last);
        assembly << instructionSet->lea(memoryOperand(last), scratch);
        assembly << ("mov r9, " + scratch.getName());
        // inGpSave: lastAddr >= save && lastAddr < save+48
        assembly << std::string("cmp r9, r12");
        assembly << instructionSet->jb(L_not_in_gp);
        assembly << std::string("lea rcx, [r12 + 48]");
        assembly << std::string("cmp r9, rcx");
        assembly << instructionSet->jae(L_not_in_gp);
        // gp_offset = lastAddr + 8 - save
        assembly << std::string("lea ecx, [r9 + 8]");
        assembly << std::string("sub ecx, r12d");
        assembly << std::string("mov dword [r8], ecx");
        assembly << std::string("mov [r8 + 8], r13");
        assembly << instructionSet->jmp(L_done);
        assembly.label(L_not_in_gp + ":");
        assembly << std::string("mov dword [r8], 48");
        assembly << std::string("lea rax, [r9 + 8]");
        assembly << std::string("mov [r8 + 8], rax");
        assembly.label(L_done + ":");
    }

    emptyGeneralPurposeRegisters();
}

void StackMachine::vaArg(std::string apName, std::string resultName, int accessSizeBytes, bool isFloating,
        bool isSigned) {
    static int seq = 0;
    const int id = ++seq;
    const std::string L_overflow = ".Lva_arg_ov_" + std::to_string(id);
    const std::string L_load = ".Lva_arg_ld_" + std::to_string(id);

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    auto& ap = resolve(apName);
    storeInMemory(ap);
    Register& scratch = registers->getRetrievalRegister();
    if (ap.getSizeInBytes() > MACHINE_WORD_SIZE) {
        assembly << instructionSet->lea(memoryOperand(ap), scratch);
    } else {
        emitLoad(ap, scratch);
    }
    assembly << ("mov r8, " + scratch.getName());

    if (isFloating) {
        assembly << std::string("mov eax, dword [r8 + 4]");
        assembly << std::string("cmp eax, 160");
        assembly << instructionSet->ja(L_overflow);
        assembly << std::string("mov rcx, [r8 + 16]");
        assembly << std::string("add rcx, rax");
        assembly << std::string("add eax, 16");
        assembly << std::string("mov dword [r8 + 4], eax");
        assembly << instructionSet->jmp(L_load);
        assembly.label(L_overflow + ":");
        assembly << std::string("mov rcx, [r8 + 8]");
        assembly << std::string("lea rax, [rcx + 8]");
        assembly << std::string("mov [r8 + 8], rax");
        assembly.label(L_load + ":");
    } else {
        assembly << std::string("mov eax, dword [r8]");
        assembly << std::string("cmp eax, 40");
        assembly << instructionSet->ja(L_overflow);
        assembly << std::string("mov rcx, [r8 + 16]");
        assembly << std::string("add rcx, rax");
        assembly << std::string("add eax, 8");
        assembly << std::string("mov dword [r8], eax");
        assembly << instructionSet->jmp(L_load);
        assembly.label(L_overflow + ":");
        assembly << std::string("mov rcx, [r8 + 8]");
        assembly << std::string("lea rax, [rcx + 8]");
        assembly << std::string("mov [r8 + 8], rax");
        assembly.label(L_load + ":");
    }

    Register& resultReg = registers->getRetrievalRegister();
    if (accessSizeBytes == 1) {
        if (isSigned) {
            assembly << ("movsx " + resultReg.getName() + ", byte [rcx]");
        } else {
            assembly << ("movzx " + resultReg.getName() + ", byte [rcx]");
        }
    } else if (accessSizeBytes == 2) {
        if (isSigned) {
            assembly << ("movsx " + resultReg.getName() + ", word [rcx]");
        } else {
            assembly << ("movzx " + resultReg.getName() + ", word [rcx]");
        }
    } else if (accessSizeBytes == 4) {
        if (isSigned) {
            assembly << ("movsxd " + resultReg.getName() + ", dword [rcx]");
        } else {
            assembly << ("mov " + reg32Name(resultReg) + ", dword [rcx]");
        }
    } else {
        assembly << ("mov " + resultReg.getName() + ", [rcx]");
    }
    bindResult(resultReg, resolve(resultName));
    storeInMemory(resolve(resultName));
    emptyGeneralPurposeRegisters();
}

void StackMachine::vaCopy(std::string dstName, std::string srcName) {
    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    auto loadTagPtr = [this](const std::string& name, const char* destReg) {
        auto& sym = resolve(name);
        storeInMemory(sym);
        Register& scratch = registers->getRetrievalRegister();
        if (sym.getSizeInBytes() > MACHINE_WORD_SIZE) {
            assembly << instructionSet->lea(memoryOperand(sym), scratch);
        } else {
            emitLoad(sym, scratch);
        }
        assembly << (std::string("mov ") + destReg + ", " + scratch.getName());
    };
    loadTagPtr(dstName, "rdi");
    loadTagPtr(srcName, "rsi");
    assembly << std::string("mov rax, [rsi]");
    assembly << std::string("mov [rdi], rax");
    assembly << std::string("mov rax, [rsi + 8]");
    assembly << std::string("mov [rdi + 8], rax");
    assembly << std::string("mov rax, [rsi + 16]");
    assembly << std::string("mov [rdi + 16], rax");
    emptyGeneralPurposeRegisters();
}


} // namespace codegen
