#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "SysVCallConv.h"

namespace codegen {

void StackMachine::createVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
        std::vector<std::string>& vaXmmHome) {
    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        std::string slotName = "__va_reg_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), ValueKind::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ slotName, slot });
        vaGpHome[i] = slotName;
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
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

void StackMachine::vaStart(std::string apName) {
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

void StackMachine::loadVaArgPiece(Register& addr, int byteOffset, Value& result, int eightbyte,
        Register& wordReg, const std::vector<Register*>& extraExclude) {
    if (result.getClassification().gprExtend != type::sysv::GprExtend::None) {
        emitClassifiedLoad(MemoryOperand::at(addr, byteOffset), wordReg, result);
        storeWord(wordReg, result, 0, extraExclude);
        return;
    }
    assembly << instructionSet->load(MemoryOperand::at(addr, byteOffset), wordReg, 8, false);
    storeWord(wordReg, result, eightbyte, extraExclude);
}

void StackMachine::alignAddressUp(Register& addr, int align, const std::vector<Register*>& live) {
    if (align <= 1) {
        return;
    }
    assembly << instructionSet->add(addr, align - 1, GprWidth::W64);
    std::vector<Register*> exclude = live;
    exclude.push_back(&addr);
    Register& mask = get64BitRegisterExcluding(exclude);
    assembly << instructionSet->mov(std::to_string(-align), mask);
    assembly << instructionSet->and_(mask, addr, GprWidth::W64);
}

void StackMachine::vaArg(std::string apName, std::string resultName) {
    const int id = ++vaArgSeq;
    const std::string L_overflow = ".Lva_arg_ov_" + std::to_string(id);
    const std::string L_done = ".Lva_arg_ld_" + std::to_string(id);

    spillGeneralPurposeRegisters();
    emptyGeneralPurposeRegisters();

    Value& result = resolve(resultName);
    const type::sysv::Classification cls = result.getClassification();
    const int needGp = type::sysv::integerEightbytes(cls);
    const int needSse = type::sysv::sseEightbytes(cls);
    const int words = wordCount(result);

    Register& tag = get64BitRegister();
    loadVaListTagPointer(apName, tag);
    Register& offsetReg = get64BitRegisterExcluding(tag);
    Register& addrReg = get64BitRegisterExcluding(std::vector<Register*> { &tag, &offsetReg });
    Register& wordReg = get64BitRegisterExcluding(std::vector<Register*> { &tag, &offsetReg, &addrReg });
    const std::vector<Register*> live { &tag, &offsetReg, &addrReg };

    storeInMemory(result);

    if (cls.inRegisters()) {
        auto fitsSaveArea = [&](int offsetField, int limit) {
            assembly << instructionSet->load(MemoryOperand::at(tag, offsetField), offsetReg, 4, false);
            assembly << instructionSet->cmp(offsetReg, limit, GprWidth::W64);
            assembly << instructionSet->ja(L_overflow);
        };
        auto copyFromSaveArea = [&](int offsetField, int stride, bool (*match)(type::sysv::Class)) {
            assembly << instructionSet->load(MemoryOperand::at(tag, offsetField), offsetReg, 4, false);
            assembly << instructionSet->load(MemoryOperand::at(tag, 16), addrReg, 8, false);
            assembly << instructionSet->add(offsetReg, addrReg, GprWidth::W64);
            int copied = 0;
            for (int i = 0; i < cls.count; ++i) {
                if (!match(cls.eightbytes[static_cast<std::size_t>(i)])) {
                    continue;
                }
                loadVaArgPiece(addrReg, copied * stride, result, i, wordReg, live);
                ++copied;
            }
            assembly << instructionSet->load(MemoryOperand::at(tag, offsetField), offsetReg, 4, false);
            assembly << instructionSet->add(offsetReg, copied * stride, GprWidth::W64);
            assembly << instructionSet->store(offsetReg, MemoryOperand::at(tag, offsetField), 4);
        };
        if (needGp > 0) {
            fitsSaveArea(0, SYSV_GP_SAVE_SIZE - needGp * MACHINE_WORD_SIZE);
        }
        if (needSse > 0) {
            fitsSaveArea(4, SYSV_GP_SAVE_SIZE
                    + (static_cast<int>(SYSV_SSE_ARG_REGS) - needSse) * SYSV_XMM_SAVE_STRIDE);
        }
        if (needGp > 0) {
            copyFromSaveArea(0, MACHINE_WORD_SIZE, type::sysv::isInteger);
        }
        if (needSse > 0) {
            copyFromSaveArea(4, SYSV_XMM_SAVE_STRIDE, type::sysv::isSse);
        }
        assembly << instructionSet->jmp(L_done);
        assembly.label(instructionSet->label(L_overflow));
    }

    assembly << instructionSet->load(MemoryOperand::at(tag, 8), addrReg, 8, false);
    if (result.getClassification().alignBytes > MACHINE_WORD_SIZE) {
        alignAddressUp(addrReg, type::object_abi::STACK_ALIGNMENT,
                std::vector<Register*> { &tag, &offsetReg, &wordReg });
    }
    assembly << instructionSet->lea(MemoryOperand::at(addrReg, words * MACHINE_WORD_SIZE), offsetReg);
    assembly << instructionSet->store(offsetReg, MemoryOperand::at(tag, 8), 8);
    for (int w = 0; w < words; ++w) {
        loadVaArgPiece(addrReg, w * MACHINE_WORD_SIZE, result, w, wordReg, live);
    }
    assembly.label(instructionSet->label(L_done));
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
