#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <stdexcept>
#include <string>

namespace codegen {

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
}

void StackMachine::fillUnusedVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
        std::vector<std::string>& vaXmmHome) {
    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        if (!vaGpHome[i].empty()) {
            continue;
        }
        std::string slotName = "__va_reg_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), Type::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ slotName, slot });
        vaGpHome[i] = slotName;
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        if (!vaXmmHome[i].empty()) {
            continue;
        }
        std::string slotName = "__va_xmm_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + vaGpSlots + static_cast<int>(i) * vaXmmWordsEach,
                Type::INTEGRAL, MACHINE_WORD_SIZE * vaXmmWordsEach };
        scopeValues.insert({ slotName, slot });
        vaXmmHome[i] = slotName;
    }
}

void StackMachine::dumpVariadicSaveArea(const std::vector<std::string>& vaGpHome,
        const std::vector<std::string>& vaXmmHome) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        assembly << instructionSet->mov(*integerArgRegs[i], memoryOperand(resolve(vaGpHome[i])));
    }
    Register& tmp = registers->getRetrievalRegister();
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
        xmmToGpr(static_cast<int>(i), tmp, false);
        emitStore(tmp, resolve(vaXmmHome[i]));
    }
    emptyGeneralPurposeRegisters();
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
    if (!variadicFrame) {
        throw std::logic_error { "va_start in non-variadic procedure" };
    }
    const VariadicFrame& frame = *variadicFrame;
    if (lastStorageName.empty()) {
        lastStorageName = frame.lastNamedFormal;
    }

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Register& tag = get64BitRegister();
    loadVaListTagPointer(apName, tag);
    Register& scratch = get64BitRegisterExcluding(tag);

    assembly << instructionSet->mov(std::to_string(frame.namedGpOffset), scratch);
    assembly << instructionSet->movDword(scratch, MemoryOperand::at(tag, 0));
    assembly << instructionSet->mov(std::to_string(frame.namedFpOffset), scratch);
    assembly << instructionSet->movDword(scratch, MemoryOperand::at(tag, 4));

    assembly << instructionSet->lea(memoryOperand(frame.regSave), scratch);
    assembly << instructionSet->mov(scratch, MemoryOperand::at(tag, 16));

    if (frame.lastFormalOnStack && !lastStorageName.empty()) {
        auto& last = resolve(lastStorageName);
        storeInMemory(last);
        assembly << instructionSet->lea(memoryOperand(last), scratch);
        const int lastBytes = type::object_abi::valueWords(last.getSizeInBytes()) * MACHINE_WORD_SIZE;
        assembly << instructionSet->lea(MemoryOperand::at(scratch, lastBytes), scratch);
        assembly << instructionSet->mov(scratch, MemoryOperand::at(tag, 8));
    } else {
        assembly << instructionSet->lea(memoryOperand(frame.overflow), scratch);
        assembly << instructionSet->mov(scratch, MemoryOperand::at(tag, 8));
    }
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
    const int step = isFloating ? 16 : 8;
    const int limit = isFloating ? SYSV_FP_OFFSET_LIMIT : SYSV_GP_OFFSET_LIMIT;

    assembly << instructionSet->movDword(MemoryOperand::at(tag, offsetField), offsetReg);
    assembly << instructionSet->cmp(offsetReg, limit);
    assembly << instructionSet->jg(L_overflow);
    assembly << instructionSet->mov(MemoryOperand::at(tag, 16), addrReg);
    assembly << instructionSet->add(offsetReg, addrReg);
    assembly << instructionSet->add(offsetReg, step);
    assembly << instructionSet->movDword(offsetReg, MemoryOperand::at(tag, offsetField));
    assembly << instructionSet->jmp(L_load);
    assembly.label(instructionSet->label(L_overflow));
    assembly << instructionSet->mov(MemoryOperand::at(tag, 8), addrReg);
    assembly << instructionSet->lea(MemoryOperand::at(addrReg, 8), offsetReg);
    assembly << instructionSet->mov(offsetReg, MemoryOperand::at(tag, 8));
    assembly.label(instructionSet->label(L_load));

    // addrReg holds the value address (reg_save+old_offset or old overflow pointer).
    Register& resultReg = get64BitRegisterExcluding(std::vector<Register*> { &addrReg, &tag });
    if (isFloating || accessSizeBytes >= 8) {
        assembly << instructionSet->mov(MemoryOperand::at(addrReg, 0), resultReg);
    } else if (accessSizeBytes <= 1) {
        assembly << instructionSet->loadByteSignExtend(addrReg, resultReg);
    } else if (isSigned) {
        assembly << instructionSet->loadDwordSignExtend(addrReg, resultReg);
    } else {
        assembly << instructionSet->movDword(MemoryOperand::at(addrReg, 0), resultReg);
    }
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
    assembly << instructionSet->mov(MemoryOperand::at(src, 0), word);
    assembly << instructionSet->mov(word, MemoryOperand::at(dst, 0));
    assembly << instructionSet->mov(MemoryOperand::at(src, 8), word);
    assembly << instructionSet->mov(word, MemoryOperand::at(dst, 8));
    assembly << instructionSet->mov(MemoryOperand::at(src, 16), word);
    assembly << instructionSet->mov(word, MemoryOperand::at(dst, 16));
    emptyGeneralPurposeRegisters();
}

void StackMachine::vaEnd() {
}

} // namespace codegen
