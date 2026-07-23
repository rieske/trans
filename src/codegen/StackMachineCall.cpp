#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "ObjectAbi.h"

namespace codegen {

void StackMachine::procedureArgument(std::string argumentName) {
    auto argument = &resolve(argumentName);
    // Multi-word values (structs) always travel on the stack.
    if (wordCount(*argument) != 1) {
        stackArguments.insert(stackArguments.begin(), argument);
        return;
    }
    // Defer GP/xmm classification until callProcedure (pure SysV packing).
    argumentSequence.push_back(argument);
}

int StackMachine::emitCallArguments(std::size_t firstReg) {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t maxIntArgs = integerArgRegs.size() > firstReg
            ? integerArgRegs.size() - firstReg
            : 0;
    constexpr std::size_t maxFloatArgs = 8;

    // Pure System V AMD64: ints pack tightly in GP; floats only in xmm0..xmm7.
    // Do not place float bits in GP slots - that displaces following ints and
    // breaks SysV va_list (git strbuf_addf "%.6f code:%d" with non-zero code).
    std::vector<Value*> gpSlots;
    std::vector<Value*> xmmArgs;

    for (Value* argument : argumentSequence) {
        if (argument->getType() == Type::FLOATING) {
            if (xmmArgs.size() < maxFloatArgs) {
                xmmArgs.push_back(argument);
            } else {
                stackArguments.insert(stackArguments.begin(), argument);
            }
        } else if (gpSlots.size() < maxIntArgs) {
            gpSlots.push_back(argument);
        } else {
            stackArguments.insert(stackArguments.begin(), argument);
        }
    }
    argumentSequence.clear();

    // Load integer GP args before stack pushes so RSP-relative temps still resolve.
    // assignRegisterToSymbol uses loadWithoutBinding, so arg regs are not marked
    // occupied; pushProcedureArgument must not reuse them as scratch.
    for (std::size_t i = 0; i < gpSlots.size(); ++i) {
        assignRegisterToSymbol(*integerArgRegs[firstReg + i], *gpSlots[i]);
    }
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();

    // Re-materialize integer args after spill.
    for (std::size_t i = 0; i < gpSlots.size(); ++i) {
        assignRegisterToSymbol(*integerArgRegs[firstReg + i], *gpSlots[i]);
    }

    int argumentOffset{0};
    int stackWords = 0;
    for (auto argument : stackArguments) {
        stackWords += wordCount(*argument);
    }
    // System V AMD64: RSP must be 16-byte aligned before call.
    if (stackWords % 2 == 1) {
        assembly << instructionSet->sub(registers->getStackPointer(), MACHINE_WORD_SIZE);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    for (auto argument : stackArguments) {
        argumentOffset += pushProcedureArgument(*argument, argumentOffset);
    }

    // Floating args: IEEE bits into xmm0..xmm7 only (SysV).
    static const char* const xmmNames[] = {
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    };
    for (std::size_t fi = 0; fi < xmmArgs.size(); ++fi) {
        Value& fv = *xmmArgs[fi];
        Register& tmp = get64BitRegisterExcluding(integerArgRegs);
        if (residesInMemory(fv)) {
            Address home = addressOf(fv);
            if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer && argumentOffset) {
                Address adjusted = Address::frame(FrameBase::StackPointer,
                        home.offsetBytes() + argumentOffset, home.sizeBytes());
                assembly << instructionSet->mov(memoryOperand(adjusted), tmp);
            } else {
                emitLoad(fv, tmp);
            }
        } else {
            Register& cur = fv.getAssignedRegister();
            if (&cur != &tmp) {
                assembly << instructionSet->mov(cur, tmp);
            }
        }
        assembly << (std::string("movq ") + xmmNames[fi] + ", " + tmp.getName());
    }

    stackArguments.clear();
    // AL = number of vector registers used (System V variadic).
    // Zero via xor so both AT&T and Intel backends emit dialect-correct code;
    // non-zero still uses NASM-style mov (production is IntelInstructionSet).
    if (xmmArgs.empty()) {
        Register& rax = registers->getMultiplicationRegister();
        assembly << instructionSet->xor_(rax, rax);
    } else {
        assembly << ("mov eax, " + std::to_string(xmmArgs.size()));
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
        assembly << instructionSet->call(target.name);
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
            Address home = addressOf(targetValue);
            if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer && argumentOffset) {
                Address adjusted = Address::frame(FrameBase::StackPointer,
                        home.offsetBytes() + argumentOffset, home.sizeBytes());
                assembly << instructionSet->mov(memoryOperand(adjusted), targetReg);
            } else {
                emitLoad(targetValue, targetReg);
            }
        }
        // AL already set by emitCallArguments.
        assembly << instructionSet->call(targetReg.getName());
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
            if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer) {
                // Prior pushes in this argument (and earlier args) have moved RSP.
                const int pushedInThisArg = (words - 1 - w) * MACHINE_WORD_SIZE;
                byteOff += argumentOffset + pushedInThisArg;
            }
            if (home.isGlobal()) {
                Register& addr = get64BitRegisterExcluding(reg, argRegs);
                assembly << instructionSet->lea(memoryOperand(home), addr);
                if (byteOff) {
                    assembly << instructionSet->add(addr, byteOff);
                }
                assembly << instructionSet->mov(MemoryOperand::at(addr, 0), reg);
            } else {
                Address wordHome = Address::frame(home.frameBase(), home.offsetBytes() + byteOff, MACHINE_WORD_SIZE);
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
    // Pop before loading the return value: __trans_va_pop_areas may clobber rax.
    // Restores the caller's save-area stack entry for nested variadic
    // (run_commit_hook -> strvec_pushf -> return -> outer va_start).
    // Spill first so a register-held return value survives the call.
    if (procedureIsVariadic) {
        spillGeneralPurposeRegisters();
        assembly << instructionSet->call("__trans_va_pop_areas");
        emptyGeneralPurposeRegisters();
    }
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        const int words = wordCount(returnSymbol);
        if (object_abi::valueNeedsMemoryReturn(returnSymbol.getSizeInBytes()) && !sretSymbolName.empty()) {
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
        } else if (returnSymbol.getType() == Type::FLOATING) {
            // System V: scalar float/double returns in xmm0.
            Register& tmp = registers->getRetrievalRegister();
            loadWord(returnSymbol, 0, tmp);
            assembly << (std::string("movq xmm0, ") + tmp.getName());
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

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value& returnSymbol = resolve(returnSymbolName);
    const int words = wordCount(returnSymbol);
    // Memory return: callee already wrote into the destination via sret.
    if (object_abi::valueNeedsMemoryReturn(returnSymbol.getSizeInBytes())) {
        return;
    }
    Register& ret = registers->getRetrievalRegister();
    // System V: floating returns arrive in xmm0 (git strtod / atof).
    if (returnSymbol.getType() == Type::FLOATING) {
        assembly << (std::string("movq ") + ret.getName() + ", xmm0");
        storeWord(ret, returnSymbol, 0);
        return;
    }
    // System V leaves upper bits of rax undefined for narrow returns. Sign-extend
    // signed ints so -1 / EOF compare equal to the constant -1 (git getc); zero-extend
    // unsigned so high-bit values keep logical >> (git sha256 uint32_t returns).
    if (returnSymbol.getType() == Type::INTEGRAL) {
        const int size = returnSymbol.getSizeInBytes();
        const bool isSigned = returnSymbol.isSigned();
        if (size == 1) {
            if (isSigned) {
                assembly << ("movsx " + ret.getName() + ", " + reg8Name(ret));
            } else {
                assembly << ("movzx " + ret.getName() + ", " + reg8Name(ret));
            }
        } else if (size == 2) {
            if (isSigned) {
                assembly << ("movsx " + ret.getName() + ", " + reg16Name(ret));
            } else {
                assembly << ("movzx " + ret.getName() + ", " + reg16Name(ret));
            }
        } else if (size == 4) {
            if (isSigned) {
                assembly << ("movsxd " + ret.getName() + ", " + reg32Name(ret));
            } else {
                assembly << ("mov " + reg32Name(ret) + ", " + reg32Name(ret));
            }
        }
    }
    storeWord(ret, returnSymbol, 0);
    if (words >= 2) {
        storeWord(registers->getRemainderRegister(), returnSymbol, 1);
    }
}


} // namespace codegen
