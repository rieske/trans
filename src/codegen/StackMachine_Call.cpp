#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
}

namespace codegen {

void StackMachine::procedureArgument(std::string argumentName) {
    argumentSequence.push_back(&resolve(argumentName));
}

void StackMachine::leaFrameOrGlobal(Value& symbol, Register& dest, int spDelta) {
    assembly << instructionSet->lea(homeOperand(symbol, spDelta), dest);
}

void StackMachine::emitGprExtend(type::sysv::GprExtend ext, int size, const MemoryOperand& source, Register& dest) {
    const bool sign = ext == type::sysv::GprExtend::Sign;
    if (size <= 1) {
        assembly << (sign ? instructionSet->loadByteSignExtend(source, dest)
                          : instructionSet->loadByteZeroExtend(source, dest));
    } else if (size <= 2) {
        assembly << (sign ? instructionSet->loadWordSignExtend(source, dest)
                          : instructionSet->loadWordZeroExtend(source, dest));
    } else if (sign) {
        assembly << instructionSet->loadDwordSignExtend(source, dest);
    } else {
        assembly << instructionSet->movDword(source, dest);
    }
}

MemoryOperand StackMachine::homeOperand(const Value& symbol, int spDelta) const {
    Address home = addressOf(symbol);
    if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer && spDelta) {
        home = Address::frame(FrameBase::StackPointer, home.offsetBytes() + spDelta, home.sizeBytes());
    }
    return memoryOperand(home);
}

void StackMachine::storeObject(Register& source, const MemoryOperand& dest, int n) {
    if (n == 1) {
        assembly << instructionSet->storeByte(source, dest);
    } else if (n == 2) {
        assembly << instructionSet->storeWord(source, dest);
    } else if (n == 4) {
        assembly << instructionSet->movDword(source, dest);
    } else {
        assembly << instructionSet->mov(source, dest);
    }
}

void StackMachine::loadWord(Value& symbol, int wordIndex, Register& dest, int spDelta,
        std::vector<Register*> extraExclude) {
    const type::sysv::GprExtend ext = symbol.getClassification().gprExtend;
    if (wordIndex == 0 && ext != type::sysv::GprExtend::None) {
        if (!symbol.isStored()) {
            Register& held = symbol.getAssignedRegister();
            storeObject(held, homeOperand(symbol, spDelta), symbol.getSizeInBytes());
            held.free();
        }
        emitGprExtend(ext, symbol.getSizeInBytes(), homeOperand(symbol, spDelta), dest);
        return;
    }
    if (wordIndex == 0 && !residesInMemory(symbol) && type::object_abi::valueWords(symbol.getSizeInBytes()) == 1) {
        Register& cur = symbol.getAssignedRegister();
        if (&cur != &dest) {
            assembly << instructionSet->mov(cur, dest);
        }
        return;
    }
    Address home = addressOf(symbol);
    const int byteOff = wordIndex * MACHINE_WORD_SIZE;
    if (home.isGlobal()) {
        extraExclude.push_back(&dest);
        Register& addr = get64BitRegisterExcluding(extraExclude);
        assembly << instructionSet->lea(memoryOperand(home), addr);
        if (byteOff) {
            assembly << instructionSet->add(addr, byteOff);
        }
        assembly << instructionSet->mov(MemoryOperand::at(addr, 0), dest);
        return;
    }
    int offset = home.offsetBytes() + byteOff;
    if (home.frameBase() == FrameBase::StackPointer) {
        offset += spDelta;
    }
    Address wordHome = Address::frame(home.frameBase(), offset, MACHINE_WORD_SIZE);
    assembly << instructionSet->mov(memoryOperand(wordHome), dest);
}

void StackMachine::bindGprExtended(Value& symbol) {
    if (symbol.getClassification().gprExtend == type::sysv::GprExtend::None) {
        return;
    }
    Register& dest = get64BitRegister();
    loadWord(symbol, 0, dest);
    bindResult(dest, symbol);
}

void StackMachine::copyWords(Value& source, Value& destination) {
    const int words = type::object_abi::valueWords(destination.getSizeInBytes());
    Register& reg = get64BitRegister();
    for (int w = 0; w < words; ++w) {
        loadWord(source, w, reg);
        storeWord(reg, destination, w);
    }
}

void StackMachine::copyBytes(Register& srcBase, Register& destBase, int n,
        const std::vector<Register*>& extraExclude) {
    if (n <= 0) {
        return;
    }
    std::vector<Register*> exclude = extraExclude;
    exclude.push_back(&srcBase);
    exclude.push_back(&destBase);
    Register& tmp = get64BitRegisterExcluding(exclude);
    exclude.push_back(&tmp);
    int off = 0;
    while (off < n) {
        const int remain = n - off;
        if (remain >= MACHINE_WORD_SIZE) {
            assembly << instructionSet->mov(MemoryOperand::at(srcBase, off), tmp);
            assembly << instructionSet->mov(tmp, MemoryOperand::at(destBase, off));
            off += MACHINE_WORD_SIZE;
            continue;
        }
        if (remain >= 4) {
            assembly << instructionSet->movDword(MemoryOperand::at(srcBase, off), tmp);
            assembly << instructionSet->movDword(tmp, MemoryOperand::at(destBase, off));
            off += 4;
            continue;
        }
        if (remain >= 2) {
            assembly << instructionSet->loadWordZeroExtend(MemoryOperand::at(srcBase, off), tmp);
            assembly << instructionSet->storeWord(tmp, MemoryOperand::at(destBase, off));
            off += 2;
        } else {
            assembly << instructionSet->loadByteZeroExtend(MemoryOperand::at(srcBase, off), tmp);
            assembly << instructionSet->storeByte(tmp, MemoryOperand::at(destBase, off));
            off += 1;
        }
    }
}

void StackMachine::copyFromPointer(Register& ptr, Value& dest) {
    if (!residesInMemory(dest)) {
        storeInMemory(dest);
    }
    Register& destBase = get64BitRegisterExcluding(ptr);
    leaFrameOrGlobal(dest, destBase, 0);
    copyBytes(ptr, destBase, dest.getSizeInBytes());
}

void StackMachine::copyToPointer(Value& src, Register& ptr, int spDelta,
        std::vector<Register*> extraExclude) {
    if (!residesInMemory(src)) {
        storeInMemory(src);
    }
    extraExclude.push_back(&ptr);
    Register& srcBase = get64BitRegisterExcluding(extraExclude);
    leaFrameOrGlobal(src, srcBase, spDelta);
    copyBytes(srcBase, ptr, src.getSizeInBytes(), extraExclude);
}

void StackMachine::storeWord(Register& source, Value& symbol, int wordIndex) {
    Address home = addressOf(symbol);
    const int byteOff = wordIndex * MACHINE_WORD_SIZE;
    if (home.isGlobal()) {
        Register& addr = get64BitRegisterExcluding(source);
        assembly << instructionSet->lea(memoryOperand(home), addr);
        if (byteOff) {
            assembly << instructionSet->add(addr, byteOff);
        }
        assembly << instructionSet->mov(source, MemoryOperand::at(addr, 0));
        return;
    }
    Address wordHome = Address::frame(home.frameBase(), home.offsetBytes() + byteOff, MACHINE_WORD_SIZE);
    assembly << instructionSet->mov(source, memoryOperand(wordHome));
}

void StackMachine::loadEightbyteToXmm(Value& symbol, int eightbyte, int xmmIndex, int spDelta,
        const std::vector<Register*>& exclude) {
    Register& tmp = get64BitRegisterExcluding(exclude);
    loadWord(symbol, eightbyte, tmp, spDelta, exclude);
    gprToXmm(tmp, xmmIndex, symbol.getSizeInBytes() == 4);
}

void StackMachine::storeEightbyteFromXmm(int xmmIndex, Value& symbol, int eightbyte,
        const std::vector<Register*>& exclude) {
    Register& tmp = get64BitRegisterExcluding(exclude);
    xmmToGpr(xmmIndex, tmp, symbol.getSizeInBytes() == 4);
    storeWord(tmp, symbol, eightbyte);
}

Register& StackMachine::integerReturnReg(int integerIndex) {
    return integerIndex == 0 ? registers->getRetrievalRegister() : registers->getRemainderRegister();
}

std::vector<Register*> StackMachine::integerReturnRegs() {
    return { &integerReturnReg(0), &integerReturnReg(1) };
}

int StackMachine::emitCallArguments(std::size_t firstReg) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();

    struct RegArg {
        Value* value;
        SysVArgAssignment asgn;
    };
    std::vector<RegArg> registerArguments;
    std::vector<Value*> stackArguments;
    SysVArgCounts counts;
    counts.integerRegs = firstReg;
    for (Value* argument : argumentSequence) {
        const SysVArgAssignment asgn = assignSysVArg(argument->getClassification(), counts,
                integerArgRegs.size());
        if (asgn.onStack) {
            stackArguments.push_back(argument);
        } else {
            registerArguments.push_back({ argument, asgn });
        }
    }
    argumentSequence.clear();

    std::vector<Register*> gpArgRegs(integerArgRegs.begin(), integerArgRegs.end());
    for (const auto& ra : registerArguments) {
        for (int i = 0; i < ra.asgn.count; ++i) {
            if (ra.asgn.slots[static_cast<std::size_t>(i)] != SysVArgSlot::IntegerReg) {
                continue;
            }
            Register& dest = *integerArgRegs[ra.asgn.indices[static_cast<std::size_t>(i)]];
            storeRegisterValue(dest);
            loadWord(*ra.value, i, dest, 0, gpArgRegs);
        }
    }
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();
    std::vector<SysVStackArg> stackSpecs;
    stackSpecs.reserve(stackArguments.size());
    for (Value* argument : stackArguments) {
        stackSpecs.push_back({ argument->getSizeInBytes(), argument->getClassification().alignBytes });
    }
    const SysVStackLayout stackLayout = layoutSysVStackArgs(stackSpecs);
    const int argumentOffset = stackLayout.totalBytes;
    if (argumentOffset) {
        assembly << instructionSet->sub(registers->getStackPointer(), argumentOffset);
    }
    Register& rsp = registers->getStackPointer();
    for (std::size_t i = 0; i < stackArguments.size(); ++i) {
        Value& argument = *stackArguments[i];
        const int slotOff = stackLayout.slots[i].offsetBytes;
        const int n = argument.getSizeInBytes();
        if (n > 0 && n % MACHINE_WORD_SIZE == 0) {
            const int words = n / MACHINE_WORD_SIZE;
            for (int w = 0; w < words; ++w) {
                Register& reg = get64BitRegisterExcluding(gpArgRegs);
                loadWord(argument, w, reg, argumentOffset, gpArgRegs);
                assembly << instructionSet->mov(reg,
                        MemoryOperand::at(rsp, slotOff + w * MACHINE_WORD_SIZE));
            }
            continue;
        }
        Register& dest = get64BitRegisterExcluding(gpArgRegs);
        assembly << instructionSet->lea(MemoryOperand::at(rsp, slotOff), dest);
        copyToPointer(argument, dest, argumentOffset, gpArgRegs);
    }

    for (const auto& ra : registerArguments) {
        for (int i = 0; i < ra.asgn.count; ++i) {
            if (ra.asgn.slots[static_cast<std::size_t>(i)] != SysVArgSlot::SseReg) {
                continue;
            }
            loadEightbyteToXmm(*ra.value, i, static_cast<int>(ra.asgn.indices[static_cast<std::size_t>(i)]),
                    argumentOffset, gpArgRegs);
        }
    }

    const std::size_t xmmCount = counts.sseRegs;
    Register& rax = registers->getMultiplicationRegister();
    if (xmmCount == 0) {
        assembly << instructionSet->xor_(rax, rax);
    } else {
        assembly << instructionSet->mov(std::to_string(xmmCount), rax);
    }
    return argumentOffset;
}

void StackMachine::emitCall(bool indirect, const std::string& target, const std::string& memoryReturnDest) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t firstReg = memoryReturnDest.empty() ? 0 : 1;

    if (!memoryReturnDest.empty()) {
        auto& dest = resolve(memoryReturnDest);
        storeInMemory(dest);
        storeRegisterValue(*integerArgRegs[0]);
        leaFrameOrGlobal(dest, *integerArgRegs[0], 0);
    }

    int argumentOffset = emitCallArguments(firstReg);
    if (!memoryReturnDest.empty()) {
        auto& dest = resolve(memoryReturnDest);
        leaFrameOrGlobal(dest, *integerArgRegs[0], argumentOffset);
    }

    if (indirect) {
        auto& targetValue = resolve(target);
        Register& targetReg = registers->getIndirectCallTargetRegister();
        if (!residesInMemory(targetValue) && type::object_abi::valueWords(targetValue.getSizeInBytes()) == 1) {
            Register& current = targetValue.getAssignedRegister();
            if (&current != &targetReg) {
                assembly << instructionSet->mov(current, targetReg);
            }
        } else {
            loadWord(targetValue, 0, targetReg, argumentOffset);
        }
        assembly << instructionSet->callIndirect(targetReg);
    } else if (isDefinedProcedure(target)) {
        assembly << instructionSet->call(target);
    } else {
        assembly << instructionSet->callPlt(target);
    }
    if (argumentOffset) {
        assembly << instructionSet->add(registers->getStackPointer(), argumentOffset);
    }
}

void StackMachine::callProcedure(std::string procedureName, std::string memoryReturnDest) {
    emitCall(false, procedureName, memoryReturnDest);
}

void StackMachine::callProcedureIndirect(std::string targetSymbolName, std::string memoryReturnDest) {
    emitCall(true, targetSymbolName, memoryReturnDest);
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const type::sysv::Classification cls = returnSymbol.getClassification();
        if (!sretSymbolName.empty()) {
            Register& rax = registers->getRetrievalRegister();
            Register& sretHold = get64BitRegisterExcluding(rax);
            loadWord(resolve(sretSymbolName), 0, sretHold);
            copyToPointer(returnSymbol, sretHold);
            if (&sretHold != &rax) {
                assembly << instructionSet->mov(sretHold, rax);
            }
        } else if (cls.inRegisters()) {
            const std::vector<Register*> retRegs = integerReturnRegs();
            int gpI = 0;
            int sseI = 0;
            for (int i = 0; i < cls.count; ++i) {
                if (type::sysv::isInteger(cls.eightbytes[static_cast<std::size_t>(i)])) {
                    loadWord(returnSymbol, i, integerReturnReg(gpI), 0, retRegs);
                    ++gpI;
                } else {
                    loadEightbyteToXmm(returnSymbol, i, sseI, 0, retRegs);
                    ++sseI;
                }
            }
        } else if (type::sysv::isComplexX87(cls)) {
            emitComplexX87Load(returnSymbol);
        } else if (cls.hasX87()) {
            storeInMemory(returnSymbol);
            assembly << instructionSet->loadX87(memoryOperand(returnSymbol));
        }
    }
    popCalleeSavedRegisters();
    assembly << instructionSet->leave();
    assembly << instructionSet->ret();
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName, bool memoryReturn) {
    if (memoryReturn) {
        return;
    }
    Value& returnSymbol = resolve(returnSymbolName);
    const type::sysv::Classification cls = returnSymbol.getClassification();
    if (cls.inRegisters()) {
        const std::vector<Register*> retRegs = integerReturnRegs();
        int gpI = 0;
        int sseI = 0;
        for (int i = 0; i < cls.count; ++i) {
            if (type::sysv::isInteger(cls.eightbytes[static_cast<std::size_t>(i)])) {
                storeWord(integerReturnReg(gpI), returnSymbol, i);
                bindGprExtended(returnSymbol);
                ++gpI;
            } else {
                storeEightbyteFromXmm(sseI, returnSymbol, i, retRegs);
                ++sseI;
            }
        }
        return;
    }
    if (type::sysv::isComplexX87(cls)) {
        emitComplexX87Store(returnSymbol);
        return;
    }
    if (cls.hasX87()) {
        storeInMemory(returnSymbol);
        assembly << instructionSet->storeX87(memoryOperand(returnSymbol));
    }
}

} // namespace codegen
