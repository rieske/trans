#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "SysVCallConv.h"

namespace codegen {

void StackMachine::fillUnusedVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
        std::vector<std::string>& vaXmmHome) {
    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        if (!vaGpHome[i].empty()) {
            continue;
        }
        std::string slotName = "__va_reg_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), ValueKind::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ slotName, slot });
        vaGpHome[i] = slotName;
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        if (!vaXmmHome[i].empty()) {
            continue;
        }
        std::string slotName = "__va_xmm_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + vaGpSlots + static_cast<int>(i) * vaXmmWordsEach,
                ValueKind::INTEGRAL, MACHINE_WORD_SIZE * vaXmmWordsEach };
        scopeValues.insert({ slotName, slot });
        vaXmmHome[i] = slotName;
    }
}

void StackMachine::dumpVariadicSaveArea(const std::vector<std::string>& vaGpHome,
        const std::vector<std::string>& vaXmmHome) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        auto& home = resolve(vaGpHome[i]);
        assembly << instructionSet->mov(*integerArgRegs[i], memoryOperand(addressOf(home)));
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        assembly << instructionSet->sseXmmToMem(static_cast<int>(i),
                memoryOperand(addressOf(resolve(vaXmmHome[i]))));
    }
}

void StackMachine::loadVaListTagPointer(const std::string& apName, Register& dest) {
    auto& ap = resolve(apName);
    storeInMemory(ap);
    if (ap.getSizeInBytes() > MACHINE_WORD_SIZE) {
        assembly << instructionSet->lea(memoryOperand(ap), dest);
    } else {
        emitLoad(ap, dest);
    }
}

void StackMachine::vaStart(std::string apName, std::string lastStorageName) {
    (void)lastStorageName;
    if (!variadicFrame) {
        throw std::logic_error { "va_start in non-variadic procedure" };
    }
    const VariadicFrame& frame = *variadicFrame;

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register& tag = get64BitRegister();
    loadVaListTagPointer(apName, tag);
    Register& scratch = get64BitRegisterExcluding(tag);

    assembly << instructionSet->storeImm(MemoryOperand::at(tag, 0), frame.namedGpOffset, 4);
    assembly << instructionSet->storeImm(MemoryOperand::at(tag, 4), frame.namedFpOffset, 4);
    assembly << instructionSet->lea(memoryOperand(frame.regSave), scratch);
    assembly << instructionSet->store(scratch, MemoryOperand::at(tag, 16), 8);
    assembly << instructionSet->lea(memoryOperand(frame.overflow), scratch);
    assembly << instructionSet->store(scratch, MemoryOperand::at(tag, 8), 8);
    emptyGeneralPurposeRegisters();
}

void StackMachine::vaArg(std::string apName, std::string resultName, int accessSizeBytes, bool isFloating,
        bool isSigned) {
    const int id = ++vaArgSeq;
    const std::string L_overflow = ".Lva_arg_ov_" + std::to_string(id);
    const std::string L_load = ".Lva_arg_ld_" + std::to_string(id);

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register& tag = get64BitRegister();
    loadVaListTagPointer(apName, tag);
    Register& offsetReg = get64BitRegisterExcluding(tag);
    Register& addrReg = get64BitRegisterExcluding(std::vector<Register*> { &tag, &offsetReg });

    const int offsetField = isFloating ? 4 : 0;
    const int step = isFloating ? SYSV_XMM_SAVE_STRIDE : MACHINE_WORD_SIZE;
    const int limit = isFloating ? SYSV_FP_OFFSET_LIMIT : SYSV_GP_OFFSET_LIMIT;

    assembly << instructionSet->load(MemoryOperand::at(tag, offsetField), offsetReg, 4, false);
    assembly << instructionSet->cmp(offsetReg, limit);
    assembly << instructionSet->ja(L_overflow);
    assembly << instructionSet->load(MemoryOperand::at(tag, 16), addrReg, 8, false);
    assembly << instructionSet->add(offsetReg, addrReg);
    assembly << instructionSet->add(offsetReg, step);
    assembly << instructionSet->store(offsetReg, MemoryOperand::at(tag, offsetField), 4);
    assembly << instructionSet->jmp(L_load);
    assembly.label(instructionSet->label(L_overflow));
    assembly << instructionSet->load(MemoryOperand::at(tag, 8), addrReg, 8, false);
    assembly << instructionSet->lea(MemoryOperand::at(addrReg, 8), offsetReg);
    assembly << instructionSet->store(offsetReg, MemoryOperand::at(tag, 8), 8);
    assembly.label(instructionSet->label(L_load));

    Register& resultReg = get64BitRegisterExcluding(std::vector<Register*> { &addrReg, &tag, &offsetReg });
    assembly << instructionSet->load(MemoryOperand::at(addrReg, 0), resultReg,
            accessWidth(accessSizeBytes), isSigned);
    bindResult(resultReg, resolve(resultName));
    storeInMemory(resolve(resultName));
    emptyGeneralPurposeRegisters();
}

void StackMachine::vaCopy(std::string dstName, std::string srcName) {
    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register& dst = get64BitRegister();
    loadVaListTagPointer(dstName, dst);
    Register& src = get64BitRegisterExcluding(dst);
    loadVaListTagPointer(srcName, src);
    Register& word = get64BitRegisterExcluding(std::vector<Register*> { &dst, &src });
    assembly << instructionSet->load(MemoryOperand::at(src, 0), word, 8, false);
    assembly << instructionSet->store(word, MemoryOperand::at(dst, 0), 8);
    assembly << instructionSet->load(MemoryOperand::at(src, 8), word, 8, false);
    assembly << instructionSet->store(word, MemoryOperand::at(dst, 8), 8);
    assembly << instructionSet->load(MemoryOperand::at(src, 16), word, 8, false);
    assembly << instructionSet->store(word, MemoryOperand::at(dst, 16), 8);
    emptyGeneralPurposeRegisters();
}

} // namespace codegen
