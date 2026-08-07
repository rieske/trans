#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

namespace codegen {

namespace {

Register R(const char* name) { return Register { name }; }

} // namespace

void StackMachine::vaStart(std::string apName, std::string lastStorageName) {
    // Fill SysV va_list tag: apName is array-of-1 object or pointer-to-tag;
    // lastStorageName empty => C23: last named formal of this procedure.
    if (lastStorageName.empty()) {
        lastStorageName = lastNamedFormalName;
    }

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register r12 = R("r12");
    Register r13 = R("r13");
    Register r8 = R("r8");
    Register rax = R("rax");

    emitNamedCall("__trans_va_get_reg_save");
    assembly << instructionSet->mov(registers->getRetrievalRegister(), r12);
    emitNamedCall("__trans_va_get_overflow");
    assembly << instructionSet->mov(registers->getRetrievalRegister(), r13);

    auto& ap = resolve(apName);
    storeInMemory(ap);
    Register& scratch = registers->getRetrievalRegister();
    if (ap.getSizeInBytes() > MACHINE_WORD_SIZE) {
        assembly << instructionSet->lea(memoryOperand(ap), scratch);
    } else {
        emitLoad(ap, scratch);
    }
    assembly << instructionSet->mov(scratch, r8);

    // gp/fp offsets come from named-formal classification (takeSysVArgSlot), not
    // from &last sitting in the GP save slice. Named floats live in the XMM slice.
    assembly << instructionSet->storeImm(MemoryOperand::at(r8, 0), vaStartState.gpOffset, 4);
    assembly << instructionSet->storeImm(MemoryOperand::at(r8, 4), vaStartState.fpOffset, 4);
    assembly << instructionSet->store(r12, MemoryOperand::at(r8, 16), 8);

    if (vaStartState.lastNamedOnStack) {
        auto& last = resolve(lastStorageName);
        storeInMemory(last);
        assembly << instructionSet->lea(memoryOperand(last), scratch);
        const int lastBytes = type::object_abi::valueWords(last.getSizeInBytes()) * MACHINE_WORD_SIZE;
        assembly << instructionSet->lea(MemoryOperand::at(scratch, lastBytes), rax);
        assembly << instructionSet->store(rax, MemoryOperand::at(r8, 8), 8);
    } else {
        assembly << instructionSet->store(r13, MemoryOperand::at(r8, 8), 8);
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

    Register r8 = R("r8");
    Register rcx = R("rcx");
    Register rax = R("rax");

    auto& ap = resolve(apName);
    storeInMemory(ap);
    Register& scratch = registers->getRetrievalRegister();
    if (ap.getSizeInBytes() > MACHINE_WORD_SIZE) {
        assembly << instructionSet->lea(memoryOperand(ap), scratch);
    } else {
        emitLoad(ap, scratch);
    }
    assembly << instructionSet->mov(scratch, r8);

    if (isFloating) {
        assembly << instructionSet->load(MemoryOperand::at(r8, 4), rax, 4, false);
        assembly << instructionSet->cmp(rax, SYSV_FP_OFFSET_LIMIT);
        assembly << instructionSet->ja(L_overflow);
        assembly << instructionSet->load(MemoryOperand::at(r8, 16), rcx, 8, false);
        assembly << instructionSet->add(rax, rcx); // rcx = reg_save + fp_offset
        assembly << instructionSet->add(rax, SYSV_XMM_SAVE_STRIDE);
        assembly << instructionSet->store(rax, MemoryOperand::at(r8, 4), 4);
        assembly << instructionSet->jmp(L_load);
        assembly.label(instructionSet->label(L_overflow));
        assembly << instructionSet->load(MemoryOperand::at(r8, 8), rcx, 8, false);
        assembly << instructionSet->lea(MemoryOperand::at(rcx, 8), rax);
        assembly << instructionSet->store(rax, MemoryOperand::at(r8, 8), 8);
        assembly.label(instructionSet->label(L_load));
    } else {
        assembly << instructionSet->load(MemoryOperand::at(r8, 0), rax, 4, false);
        assembly << instructionSet->cmp(rax, SYSV_GP_OFFSET_LIMIT);
        assembly << instructionSet->ja(L_overflow);
        assembly << instructionSet->load(MemoryOperand::at(r8, 16), rcx, 8, false);
        assembly << instructionSet->add(rax, rcx); // rcx = reg_save + gp_offset
        assembly << instructionSet->add(rax, MACHINE_WORD_SIZE);
        assembly << instructionSet->store(rax, MemoryOperand::at(r8, 0), 4);
        assembly << instructionSet->jmp(L_load);
        assembly.label(instructionSet->label(L_overflow));
        assembly << instructionSet->load(MemoryOperand::at(r8, 8), rcx, 8, false);
        assembly << instructionSet->lea(MemoryOperand::at(rcx, 8), rax);
        assembly << instructionSet->store(rax, MemoryOperand::at(r8, 8), 8);
        assembly.label(instructionSet->label(L_load));
    }

    // rcx holds the value address (reg_save+offset or old overflow pointer).
    Register& resultReg = registers->getRetrievalRegister();
    assembly << instructionSet->load(MemoryOperand::at(rcx, 0), resultReg,
            accessWidth(accessSizeBytes), isSigned);
    bindResult(resultReg, resolve(resultName));
    storeInMemory(resolve(resultName));
    emptyGeneralPurposeRegisters();
}

void StackMachine::vaCopy(std::string dstName, std::string srcName) {
    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register rdi = R("rdi");
    Register rsi = R("rsi");
    Register rax = R("rax");

    auto loadTagPtr = [this](const std::string& name, Register& dest) {
        auto& sym = resolve(name);
        storeInMemory(sym);
        Register& scratch = registers->getRetrievalRegister();
        if (sym.getSizeInBytes() > MACHINE_WORD_SIZE) {
            assembly << instructionSet->lea(memoryOperand(sym), scratch);
        } else {
            emitLoad(sym, scratch);
        }
        assembly << instructionSet->mov(scratch, dest);
    };
    loadTagPtr(dstName, rdi);
    loadTagPtr(srcName, rsi);
    assembly << instructionSet->load(MemoryOperand::at(rsi, 0), rax, 8, false);
    assembly << instructionSet->store(rax, MemoryOperand::at(rdi, 0), 8);
    assembly << instructionSet->load(MemoryOperand::at(rsi, 8), rax, 8, false);
    assembly << instructionSet->store(rax, MemoryOperand::at(rdi, 8), 8);
    assembly << instructionSet->load(MemoryOperand::at(rsi, 16), rax, 8, false);
    assembly << instructionSet->store(rax, MemoryOperand::at(rdi, 16), 8);
    emptyGeneralPurposeRegisters();
}

} // namespace codegen
