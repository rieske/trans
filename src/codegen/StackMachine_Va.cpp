#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <stdexcept>
#include <string>

namespace codegen {

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
}

void StackMachine::createVaSaveHomes(int vaSaveBaseIndex, std::vector<std::string>& vaGpHome,
        std::vector<std::string>& vaXmmHome) {
    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
        std::string slotName = "__va_reg_" + std::to_string(i);
        Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), Type::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ slotName, slot });
        vaGpHome[i] = slotName;
    }
    for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
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

void StackMachine::loadVaArgPiece(Register& addr, int byteOffset, Value& result, int eightbyte,
        Register& wordReg) {
    loadPromotedFrom(MemoryOperand::at(addr, byteOffset), result, wordReg);
    if (result.getClassification().gprExtend != type::sysv::GprExtend::None) {
        storeWord(wordReg, result, 0);
        return;
    }
    storeWord(wordReg, result, eightbyte);
}

void StackMachine::alignAddressUp(Register& addr, int align, const std::vector<Register*>& live) {
    if (align <= 1) {
        return;
    }
    assembly << instructionSet->add(addr, align - 1);
    std::vector<Register*> exclude = live;
    exclude.push_back(&addr);
    Register& mask = get64BitRegisterExcluding(exclude);
    assembly << instructionSet->mov(std::to_string(-align), mask);
    assembly << instructionSet->and_(mask, addr);
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
    const int words = type::object_abi::valueWords(result.getSizeInBytes());

    Register& tag = get64BitRegister();
    loadVaListTagPointer(apName, tag);
    Register& offsetReg = get64BitRegisterExcluding(tag);
    Register& addrReg = get64BitRegisterExcluding(std::vector<Register*> { &tag, &offsetReg });
    Register& wordReg = get64BitRegisterExcluding(std::vector<Register*> { &tag, &offsetReg, &addrReg });

    storeInMemory(result);

    if (cls.inRegisters()) {
        // Check both banks before either offset advances.
        auto fitsSaveArea = [&](int offsetField, int limit) {
            assembly << instructionSet->movDword(MemoryOperand::at(tag, offsetField), offsetReg);
            assembly << instructionSet->cmp(offsetReg, limit);
            assembly << instructionSet->jg(L_overflow);
        };
        auto copyFromSaveArea = [&](int offsetField, int stride, bool (*match)(type::sysv::Class)) {
            assembly << instructionSet->movDword(MemoryOperand::at(tag, offsetField), offsetReg);
            assembly << instructionSet->mov(MemoryOperand::at(tag, 16), addrReg);
            assembly << instructionSet->add(offsetReg, addrReg);
            int copied = 0;
            for (int i = 0; i < cls.count; ++i) {
                if (!match(cls.eightbytes[static_cast<std::size_t>(i)])) {
                    continue;
                }
                loadVaArgPiece(addrReg, copied * stride, result, i, wordReg);
                ++copied;
            }
            assembly << instructionSet->movDword(MemoryOperand::at(tag, offsetField), offsetReg);
            assembly << instructionSet->add(offsetReg, copied * stride);
            assembly << instructionSet->movDword(offsetReg, MemoryOperand::at(tag, offsetField));
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

    assembly << instructionSet->mov(MemoryOperand::at(tag, 8), addrReg);
    if (result.getClassification().alignBytes > MACHINE_WORD_SIZE) {
        alignAddressUp(addrReg, type::object_abi::STACK_ALIGNMENT,
                std::vector<Register*> { &tag, &offsetReg, &wordReg });
    }
    assembly << instructionSet->lea(MemoryOperand::at(addrReg, words * MACHINE_WORD_SIZE), offsetReg);
    assembly << instructionSet->mov(offsetReg, MemoryOperand::at(tag, 8));
    for (int w = 0; w < words; ++w) {
        loadVaArgPiece(addrReg, w * MACHINE_WORD_SIZE, result, w, wordReg);
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
