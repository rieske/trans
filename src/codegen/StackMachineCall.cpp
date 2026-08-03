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

    std::vector<std::pair<Value*, SysVArgPlacement>> placed;
    placed.reserve(argumentSequence.size());
    SysVArgCounts counts;
    counts.integerRegs = firstReg;
    for (Value* argument : argumentSequence) {
        placed.push_back({ argument, takeSysVArgSlot(*argument, counts, SYSV_INTEGER_ARG_REGS) });
    }
    argumentSequence.clear();

    // Load integer GP args before stack pushes so RSP-relative temps still resolve.
    // assignRegisterToSymbol uses loadWithoutBinding, so arg regs are not marked
    // occupied; pushProcedureArgument must not reuse them as scratch.
    // place.index is the GP number (sret seeds firstReg); do not recompute firstReg+i.
    for (const auto& item : placed) {
        if (item.second.slot == SysVArgSlot::IntegerReg) {
            assignRegisterToSymbol(*integerArgRegs[item.second.index], *item.first);
        }
    }
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();

    // Re-materialize integer args after spill.
    for (const auto& item : placed) {
        if (item.second.slot == SysVArgSlot::IntegerReg) {
            assignRegisterToSymbol(*integerArgRegs[item.second.index], *item.first);
        }
    }

    int argumentOffset{0};
    int stackWords = 0;
    for (const auto& item : placed) {
        if (item.second.slot == SysVArgSlot::Stack) {
            stackWords += wordCount(*item.first);
        }
    }
    // System V AMD64: RSP must be 16-byte aligned before call.
    if (stackWords % 2 == 1) {
        assembly << instructionSet->sub(registers->getStackPointer(), MACHINE_WORD_SIZE);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    for (auto it = placed.rbegin(); it != placed.rend(); ++it) {
        if (it->second.slot == SysVArgSlot::Stack) {
            argumentOffset += pushProcedureArgument(*it->first, argumentOffset);
        }
    }

    // Floating args: IEEE bits into xmm0..xmm7 only (SysV).
    for (const auto& item : placed) {
        if (item.second.slot != SysVArgSlot::SseReg) {
            continue;
        }
        Value& fv = *item.first;
        Register& tmp = get64BitRegisterExcluding(integerArgRegs);
        if (residesInMemory(fv)) {
            emitLoadFromHome(fv, tmp, argumentOffset);
        } else {
            Register& cur = fv.getAssignedRegister();
            if (&cur != &tmp) {
                assembly << instructionSet->mov(cur, tmp);
            }
        }
        gprToXmm(tmp, static_cast<int>(item.second.index), sseWidth(fv));
    }

    // AL = number of vector registers used (System V variadic).
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
        assembly << instructionSet->lea(memoryOperand(dest), *integerArgRegs[0]);
    }

    // Spills caller-saved regs (including rdi); rematerialize sret after.
    int argumentOffset = emitCallArguments(firstReg);
    if (!memoryReturnDest.empty()) {
        auto& dest = resolve(memoryReturnDest);
        assembly << instructionSet->lea(memoryOperand(dest), *integerArgRegs[0]);
    }

    if (target.kind == CallTarget::Kind::Named) {
        emitNamedCall(target.name);
    } else {
        // Load the callee address into r10 after arg setup. r10 is caller-saved and
        // not an integer argument register; using rdi/rsi/rdx/rcx/r8/r9 would
        // overwrite a live argument (git config_fn_t 4th arg is data in rcx).
        // Stack-arg pushes moved RSP: if the target lives in an RSP-relative spill
        // slot, add argumentOffset so we still load the function pointer (git
        // show_one_reflog_ent: without this, call *fn becomes call *refname).
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

int StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    const int words = wordCount(symbolToPush);
    // Never use integer arg regs as scratch: callProcedure has already loaded
    // rdi..r9 via loadWithoutBinding (unmarked), and clobbering them breaks
    // multi-arg calls (git commit_tree_extended: rcx/parents <- lea sign_commit).
    const auto& argRegs = registers->getIntegerArgumentRegisters();
    // Push high word first so the lowest address holds word 0 (matches callee RBP layout).
    for (int w = words - 1; w >= 0; --w) {
        Register& reg = get64BitRegisterExcluding(argRegs);
        if (residesInMemory(symbolToPush) || words > 1) {
            Address home = addressOf(symbolToPush);
            int byteOff = w * MACHINE_WORD_SIZE;
            // Prior pushes in this argument (and earlier args) have moved RSP.
            const int pushedInThisArg = (words - 1 - w) * MACHINE_WORD_SIZE;
            const int spDelta = (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer)
                    ? (argumentOffset + pushedInThisArg)
                    : 0;
            if (home.isGlobal()) {
                Register& addr = get64BitRegisterExcluding(reg, argRegs);
                assembly << instructionSet->lea(memoryOperand(home), addr);
                if (byteOff) {
                    assembly << instructionSet->add(addr, byteOff);
                }
                assembly << instructionSet->mov(MemoryOperand::at(addr, 0), reg);
            } else {
                Address base = homeAfterSpDelta(home, spDelta);
                Address wordHome = Address::frame(base.frameBase(), base.offsetBytes() + byteOff, MACHINE_WORD_SIZE);
                assembly << instructionSet->mov(memoryOperand(wordHome), reg);
            }
            assembly << instructionSet->push(reg);
        } else {
            assembly << instructionSet->push(symbolToPush.getAssignedRegister());
        }
    }
    return words * MACHINE_WORD_SIZE;
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const int words = wordCount(returnSymbol);
        // Trust StartProcedure memoryReturn bit (sretSymbolName set only then).
        if (!sretSymbolName.empty()) {
            // System V memory return: copy object to [sret], leave sret in rax.
            // Hold the destination pointer in a dedicated register. loadWord of a
            // global source uses a scratch for LEA and must not clobber that
            // pointer (git: return incremental_strategy; was *src=*src).
            Register& rax = registers->getRetrievalRegister();
            Register& tmp = get64BitRegisterExcluding(rax);
            Register& sretHold = get64BitRegisterExcluding(std::vector<Register*> { &rax, &tmp });
            loadWord(resolve(sretSymbolName), 0, sretHold);
            for (int w = 0; w < words; ++w) {
                Address home = addressOf(returnSymbol);
                const int byteOff = w * MACHINE_WORD_SIZE;
                if (home.isGlobal()) {
                    Register& addr = get64BitRegisterExcluding(
                            std::vector<Register*> { &tmp, &sretHold });
                    assembly << instructionSet->lea(memoryOperand(home), addr);
                    if (byteOff) {
                        assembly << instructionSet->add(addr, byteOff);
                    }
                    assembly << instructionSet->mov(MemoryOperand::at(addr, 0), tmp);
                } else {
                    loadWord(returnSymbol, w, tmp);
                }
                assembly << instructionSet->mov(tmp,
                        MemoryOperand::at(sretHold, w * MACHINE_WORD_SIZE));
            }
            // rax holds the hidden pointer (required by System V).
            if (&sretHold != &rax) {
                assembly << instructionSet->mov(sretHold, rax);
            }
        } else if (returnSymbol.getValueKind() == ValueKind::FLOATING) {
            // System V: scalar float/double returns in xmm0.
            Register& tmp = registers->getRetrievalRegister();
            if (residesInMemory(returnSymbol)) {
                emitLoad(returnSymbol, tmp);
            } else if (&tmp != &returnSymbol.getAssignedRegister()) {
                assembly << instructionSet->mov(returnSymbol.getAssignedRegister(), tmp);
            }
            gprToXmm(tmp, 0, sseWidth(returnSymbol));
        } else {
            // Up to two words returned in RAX and RDX (Point, etc.).
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
    Value& returnSymbol = resolve(returnSymbolName);
    const int words = wordCount(returnSymbol);
    // Memory return: callee already wrote into the destination via sret.
    // Trust the Retrieve bit set when Call had a non-empty memoryReturnDest
    // (same decision as productEmitsMemoryReturn at emit time) — do not re-derive
    // from size alone.
    if (memoryReturn) {
        return;
    }
    Register& ret = registers->getRetrievalRegister();
    // System V: floating returns arrive in xmm0 (git strtod / atof).
    if (returnSymbol.getValueKind() == ValueKind::FLOATING) {
        xmmToGpr(0, ret, sseWidth(returnSymbol));
        emitStore(ret, returnSymbol);
        return;
    }
    // System V leaves upper bits of rax undefined for narrow returns. Sign-extend
    // signed ints so -1 / EOF compare equal to the constant -1 (git getc); zero-extend
    // unsigned so high-bit values keep logical >> (git sha256 uint32_t returns).
    if (returnSymbol.getValueKind() == ValueKind::INTEGRAL) {
        const int size = returnSymbol.getSizeInBytes();
        if (accessWidth(size) < MACHINE_WORD_SIZE) {
            assembly << instructionSet->extend(ret, size, returnSymbol.isSigned());
        }
    }
    storeWord(ret, returnSymbol, 0);
    if (words >= 2) {
        storeWord(registers->getRemainderRegister(), returnSymbol, 1);
    }
}


} // namespace codegen
