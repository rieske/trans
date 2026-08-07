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

int StackMachine::emitCallArguments(std::size_t firstReg) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();

    std::vector<Value*> integerArguments;
    std::vector<Value*> floatingArguments;
    std::vector<Value*> stackArguments;
    SysVArgCounts counts;
    counts.integerRegs = firstReg;
    for (Value* argument : argumentSequence) {
        switch (takeSysVArgSlot(*argument, counts, integerArgRegs.size()).slot) {
        case SysVArgSlot::SseReg:
            floatingArguments.push_back(argument);
            break;
        case SysVArgSlot::IntegerReg:
            integerArguments.push_back(argument);
            break;
        case SysVArgSlot::Stack:
            stackArguments.push_back(argument);
            break;
        }
    }
    argumentSequence.clear();

    for (std::size_t i = 0; i < integerArguments.size(); ++i) {
        assignRegisterToSymbol(*integerArgRegs[firstReg + i], *integerArguments[i]);
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

    // Floating args: IEEE bits into xmm0..xmm7 only (SysV).
    for (std::size_t fi = 0; fi < floatingArguments.size(); ++fi) {
        Value& fv = *floatingArguments[fi];
        Register& tmp = get64BitRegisterExcluding(integerArgRegs);
        loadWord(fv, 0, tmp, argumentOffset);
        gprToXmm(tmp, static_cast<int>(fi), isSseFloat32(fv));
    }

    const std::size_t xmmCount = floatingArguments.size();
    // AL = number of vector registers used (System V variadic).
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
        loadWord(symbolToPush, w, reg, argumentOffset + (words - 1 - w) * MACHINE_WORD_SIZE);
        assembly << instructionSet->push(reg);
    }
    return words * MACHINE_WORD_SIZE;
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const int words = type::object_abi::valueWords(returnSymbol.getSizeInBytes());
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
        } else if (returnSymbol.getType() == Type::FLOATING) {
            Register& tmp = registers->getRetrievalRegister();
            if (residesInMemory(returnSymbol)) {
                emitLoad(returnSymbol, tmp);
            } else if (&tmp != &returnSymbol.getAssignedRegister()) {
                assembly << instructionSet->mov(returnSymbol.getAssignedRegister(), tmp);
            }
            gprToXmm(tmp, 0, isSseFloat32(returnSymbol));
        } else {
            loadWord(returnSymbol, 0, registers->getRetrievalRegister());
            if (words >= 2) {
                loadWord(returnSymbol, 1, registers->getRemainderRegister());
            }
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
    const int words = type::object_abi::valueWords(returnSymbol.getSizeInBytes());
    Register& ret = registers->getRetrievalRegister();
    if (returnSymbol.getType() == Type::FLOATING) {
        xmmToGpr(0, ret, isSseFloat32(returnSymbol));
        emitStore(ret, returnSymbol);
        return;
    }
    storeWord(ret, returnSymbol, 0);
    if (words >= 2) {
        storeWord(registers->getRemainderRegister(), returnSymbol, 1);
    }
}

} // namespace codegen
