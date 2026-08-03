#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "SysVCallConv.h"
#include "types/ObjectAbiType.h"

namespace codegen {

void StackMachine::procedureArgument(std::string argumentName) {
    argumentSequence.push_back(&resolve(argumentName));
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
        stackSpecs.push_back(stackArgOf(*argument));
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
        const int words = wordCount(argument);
        for (int w = 0; w < words; ++w) {
            Register& reg = get64BitRegisterExcluding(gpArgRegs);
            loadWord(argument, w, reg, argumentOffset, gpArgRegs);
            assembly << instructionSet->mov(reg,
                    MemoryOperand::at(rsp, slotOff + w * MACHINE_WORD_SIZE));
        }
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

    Register& rax = registers->getMultiplicationRegister();
    if (counts.sseRegs == 0) {
        assembly << instructionSet->xor_(rax, rax);
    } else {
        assembly << instructionSet->mov(std::to_string(counts.sseRegs), rax);
    }
    return argumentOffset;
}

void StackMachine::emitCall(const CallTarget& target, const std::string& memoryReturnDest) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t firstReg = memoryReturnDest.empty() ? 0 : 1;

    // System V sret: pass &dest in the first integer arg register.
    if (!memoryReturnDest.empty()) {
        auto& dest = resolve(memoryReturnDest);
        storeInMemory(dest);
        storeRegisterValue(*integerArgRegs[0]);
        leaFrameOrGlobal(dest, *integerArgRegs[0], 0);
    }

    // Spills caller-saved regs (including rdi); rematerialize sret after.
    // Outgoing stack args move RSP: dest may live in an RSP-relative spill.
    int argumentOffset = emitCallArguments(firstReg);
    if (!memoryReturnDest.empty()) {
        leaFrameOrGlobal(resolve(memoryReturnDest), *integerArgRegs[0], argumentOffset);
    }

    if (target.kind == CallTarget::Kind::Named) {
        emitNamedCall(target.name);
    } else {
        // Load the callee address into r10 after arg setup. r10 is caller-saved and
        // not an integer argument register; using rdi/rsi/rdx/rcx/r8/r9 would
        // overwrite a live argument (git config_fn_t 4th arg is data in rcx).
        // Outgoing stack args moved RSP: if the target lives in an RSP-relative
        // spill slot, add argumentOffset so we still load the function pointer
        // (git show_one_reflog_ent: without this, call *fn becomes call *refname).
        auto& targetValue = resolve(target.name);
        Register& targetReg = registers->getIndirectCallTargetRegister();
        if (!residesInMemory(targetValue)) {
            Register& current = targetValue.getAssignedRegister();
            if (&current != &targetReg) {
                assembly << instructionSet->mov(current, targetReg);
            }
        } else {
            emitLoadFromHome(targetValue, targetReg, argumentOffset);
        }
        // AL already set by emitCallArguments. callIndirect is dialect-correct (AT&T: call *%reg).
        assembly << instructionSet->callIndirect(targetReg);
    }

    if (argumentOffset) {
        assembly << instructionSet->add(registers->getStackPointer(), argumentOffset);
    }
}

void StackMachine::callProcedure(std::string procedureName, std::string memoryReturnDest) {
    emitCall(CallTarget::named(std::move(procedureName)), memoryReturnDest);
}

void StackMachine::callProcedureIndirect(std::string targetSymbolName, std::string memoryReturnDest) {
    emitCall(CallTarget::indirect(std::move(targetSymbolName)), memoryReturnDest);
}

void StackMachine::leaFrameOrGlobal(Value& symbol, Register& dest, int spDelta) {
    assembly << instructionSet->lea(
            memoryOperand(homeAfterSpDelta(addressOf(symbol), spDelta)), dest);
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const type::sysv::Classification cls = returnSymbol.getClassification();
        if (!sretSymbolName.empty()) {
            const int words = wordCount(returnSymbol);
            Register& rax = registers->getRetrievalRegister();
            Register& tmp = get64BitRegisterExcluding(rax);
            Register& sretHold = get64BitRegisterExcluding(std::vector<Register*> { &rax, &tmp });
            loadWord(resolve(sretSymbolName), 0, sretHold);
            for (int w = 0; w < words; ++w) {
                loadWord(returnSymbol, w, tmp, 0, { &sretHold });
                assembly << instructionSet->mov(tmp,
                        MemoryOperand::at(sretHold, w * MACHINE_WORD_SIZE));
            }
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
