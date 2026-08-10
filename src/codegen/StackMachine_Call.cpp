#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
}

namespace codegen {

void StackMachine::procedureArgument(std::string argumentName) {
    argumentSequence.push_back(&resolve(argumentName));
}

void StackMachine::leaFrameOrGlobal(Value& symbol, Register& dest, int spDelta) {
    Address home = addressOf(symbol);
    if (home.isGlobal()) {
        assembly << instructionSet->lea(memoryOperand(home), dest);
        return;
    }
    if (home.frameBase() == FrameBase::StackPointer && spDelta) {
        home = Address::frame(FrameBase::StackPointer, home.offsetBytes() + spDelta, home.sizeBytes());
    }
    assembly << instructionSet->lea(memoryOperand(home), dest);
}

void StackMachine::loadWord(Value& symbol, int wordIndex, Register& dest, int spDelta,
        std::vector<Register*> extraExclude) {
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

void StackMachine::copyWords(Value& source, Value& destination) {
    const int words = type::object_abi::valueWords(destination.getSizeInBytes());
    Register& reg = get64BitRegister();
    for (int w = 0; w < words; ++w) {
        loadWord(source, w, reg);
        storeWord(reg, destination, w);
    }
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

void StackMachine::storeEightbyteFromXmm(int xmmIndex, Value& symbol, int eightbyte) {
    Register& tmp = registers->getRetrievalRegister();
    xmmToGpr(xmmIndex, tmp, symbol.getSizeInBytes() == 4);
    storeWord(tmp, symbol, eightbyte);
}

Register& StackMachine::integerReturnReg(int eightbyteIndex) {
    return eightbyteIndex == 0 ? registers->getRetrievalRegister() : registers->getRemainderRegister();
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
    int argumentOffset { 0 };
    int stackWords = 0;
    for (auto argument : stackArguments) {
        stackWords += type::object_abi::valueWords(argument->getSizeInBytes());
    }
    // System V AMD64: RSP must be 16-byte aligned before call. Without stack args we are
    // aligned; each stack word is 8 bytes, so an odd count needs 8 bytes of padding.
    if (stackWords % 2 == 1) {
        assembly << instructionSet->sub(registers->getStackPointer(), MACHINE_WORD_SIZE);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    for (auto it = stackArguments.rbegin(); it != stackArguments.rend(); ++it) {
        argumentOffset += pushProcedureArgument(**it, argumentOffset);
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

int StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    const int words = type::object_abi::valueWords(symbolToPush.getSizeInBytes());
    const auto& argRegs = registers->getIntegerArgumentRegisters();
    for (int w = words - 1; w >= 0; --w) {
        Register& reg = get64BitRegisterExcluding(argRegs);
        loadWord(symbolToPush, w, reg, argumentOffset + (words - 1 - w) * MACHINE_WORD_SIZE, argRegs);
        assembly << instructionSet->push(reg);
    }
    return words * MACHINE_WORD_SIZE;
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const int words = type::object_abi::valueWords(returnSymbol.getSizeInBytes());
        const type::sysv::Classification cls = returnSymbol.getClassification();
        if (!sretSymbolName.empty()) {
            Register& rax = registers->getRetrievalRegister();
            Register& tmp = get64BitRegisterExcluding(rax);
            Register& sretHold = get64BitRegisterExcluding(std::vector<Register*> { &rax, &tmp });
            loadWord(resolve(sretSymbolName), 0, sretHold);
            for (int w = 0; w < words; ++w) {
                loadWord(returnSymbol, w, tmp, 0, { &sretHold });
                assembly << instructionSet->mov(tmp, MemoryOperand::at(sretHold, w * MACHINE_WORD_SIZE));
            }
            if (&sretHold != &rax) {
                assembly << instructionSet->mov(sretHold, rax);
            }
        } else if (cls.inRegisters()) {
            Register& rax = registers->getRetrievalRegister();
            Register& rdx = registers->getRemainderRegister();
            const std::vector<Register*> retRegs { &rax, &rdx };
            int sseI = 0;
            for (int i = 0; i < cls.count; ++i) {
                if (type::sysv::isInteger(cls.eightbytes[static_cast<std::size_t>(i)])) {
                    loadWord(returnSymbol, i, integerReturnReg(i), 0, retRegs);
                } else {
                    loadEightbyteToXmm(returnSymbol, i, sseI, 0, retRegs);
                    ++sseI;
                }
            }
        } else if (cls.hasX87()) {
            // No st(0) emit yet: first eightbyte bits in xmm0.
            loadEightbyteToXmm(returnSymbol, 0, 0, 0, {});
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
        int sseI = 0;
        for (int i = 0; i < cls.count; ++i) {
            if (type::sysv::isInteger(cls.eightbytes[static_cast<std::size_t>(i)])) {
                storeWord(integerReturnReg(i), returnSymbol, i);
            } else {
                storeEightbyteFromXmm(sseI, returnSymbol, i);
                ++sseI;
            }
        }
        return;
    }
    if (cls.hasX87()) {
        storeEightbyteFromXmm(0, returnSymbol, 0);
    }
}

} // namespace codegen
