#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "ObjectAbi.h"

namespace codegen {

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    storeInMemory(operand);
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(memoryOperand(operand), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::functionAddress(std::string functionName, std::string resultName) {
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(MemoryOperand::global(functionName), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName,
        int accessSizeBytes, bool isSigned) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);
    // Use the register returned by the load path; global pointer homes are not register-bound.
    Register& pointerRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();

    // Multi-word aggregate (struct/array > 8 bytes): copy every word from
    // [ptr+off] into the result home. A single-qword load left words 1..n as
    // stack garbage (git: r->event_jobs = cb_data.event_jobs).
    const int words = wordCount(result);
    if (words > 1) {
        // Keep the address for lvalue uses (assignment through the same access).
        Register& lvalueRegister = get64BitRegisterExcluding(pointerRegister);
        assembly << instructionSet->mov(pointerRegister, lvalueRegister);
        lvalueRegister.assign(&resolve(lvalueName));
        // Spill pointer into the lvalue temp so reloads after register pressure
        // still see the base address.
        storeInMemory(resolve(lvalueName));
        storeInMemory(result);
        for (int w = 0; w < words; ++w) {
            Register& addr = residesInMemory(resolve(lvalueName))
                    ? assignRegisterTo(resolve(lvalueName))
                    : resolve(lvalueName).getAssignedRegister();
            Register& tmp = get64BitRegisterExcluding(addr);
            assembly << instructionSet->mov(MemoryOperand::at(addr, w * MACHINE_WORD_SIZE), tmp);
            storeWord(tmp, result, w);
        }
        return;
    }

    Register& resultRegister = get64BitRegisterExcluding(pointerRegister);
    if (accessSizeBytes == 1) {
        if (isSigned) {
            assembly << ("movsx " + resultRegister.getName() + ", byte " + memoryAt(pointerRegister, 0));
        } else {
            // Zero-extend packed char so upper bits do not pollute integral uses.
            assembly << ("movzx " + resultRegister.getName() + ", byte " + memoryAt(pointerRegister, 0));
        }
    } else if (accessSizeBytes == 2) {
        if (isSigned) {
            assembly << ("movsx " + resultRegister.getName() + ", word " + memoryAt(pointerRegister, 0));
        } else {
            // Zero-extend 16-bit (unsigned short / ctype table entries).
            assembly << ("movzx " + resultRegister.getName() + ", word " + memoryAt(pointerRegister, 0));
        }
    } else if (accessSizeBytes == 4) {
        if (isSigned) {
            // Sign-extend 32-bit int so negatives stay correct in 64-bit regs.
            assembly << ("movsxd " + resultRegister.getName() + ", dword " + memoryAt(pointerRegister, 0));
        } else {
            // 32-bit mov zero-extends into the full 64-bit register (unsigned int / uint32_t).
            assembly << ("mov " + reg32Name(resultRegister) + ", dword " + memoryAt(pointerRegister, 0));
        }
    } else {
        assembly << instructionSet->mov(MemoryOperand::at(pointerRegister, 0), resultRegister);
    }
    bindResult(resultRegister, result);

    Register& lvalueRegister = get64BitRegisterExcluding(pointerRegister);
    assembly << instructionSet->mov(pointerRegister, lvalueRegister);
    lvalueRegister.assign(&resolve(lvalueName));
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);
    if (wordCount(operand) > 1 || wordCount(result) > 1) {
        copyWords(operand, result);
        return;
    }
    // Float <-> int conversion for casts (and assignments) using SSE2.
    if (operand.getType() == Type::FLOATING && result.getType() == Type::INTEGRAL) {
        Register& src = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
        Register& dst = residesInMemory(result) ? get64BitRegisterExcluding(src) : result.getAssignedRegister();
        assembly << ("movq xmm0, " + src.getName());
        assembly << ("cvttsd2si " + dst.getName() + ", xmm0");
        if (residesInMemory(result)) {
            emitStore(dst, result);
        } else {
            bindResult(dst, result);
        }
        return;
    }
    if (operand.getType() == Type::INTEGRAL && result.getType() == Type::FLOATING) {
        Register& src = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
        Register& dst = residesInMemory(result) ? get64BitRegisterExcluding(src) : result.getAssignedRegister();
        assembly << ("cvtsi2sd xmm0, " + src.getName());
        assembly << ("movq " + dst.getName() + ", xmm0");
        if (residesInMemory(result)) {
            emitStore(dst, result);
        } else {
            bindResult(dst, result);
        }
        return;
    }
    if (residesInMemory(operand) && residesInMemory(result)) {
        Register& reg = get64BitRegister();
        emitLoad(operand, reg);
        emitStore(reg, result);
    } else if (residesInMemory(operand)) {
        emitLoad(operand, result.getAssignedRegister());
    } else if (residesInMemory(result)) {
        emitStore(operand.getAssignedRegister(), result);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto& result = resolve(resultName);
    // NASM `mov r/m64, imm` only accepts a signed 32-bit immediate. Float IEEE
    // bit patterns and large integers must go through a register first.
    Register& reg = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    assembly << instructionSet->mov(constant, reg);
    if (residesInMemory(result)) {
        emitStore(reg, result);
    }
}

void StackMachine::truncate(std::string operandName, int sizeBytes, bool isSigned) {
    auto& operand = resolve(operandName);
    Register& reg = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    if (sizeBytes == 1) {
        if (isSigned) {
            assembly << ("movsx " + reg.getName() + ", " + reg8Name(reg));
        } else {
            assembly << ("movzx " + reg.getName() + ", " + reg8Name(reg));
        }
    } else if (sizeBytes == 2) {
        if (isSigned) {
            assembly << ("movsx " + reg.getName() + ", " + reg16Name(reg));
        } else {
            assembly << ("movzx " + reg.getName() + ", " + reg16Name(reg));
        }
    } else if (sizeBytes == 4) {
        if (isSigned) {
            assembly << ("movsxd " + reg.getName() + ", " + reg32Name(reg));
        } else {
            // 32-bit mov zero-extends into the full 64-bit register.
            assembly << ("mov " + reg32Name(reg) + ", " + reg32Name(reg));
        }
    } else {
        throw std::runtime_error {
                "truncate: unsupported size " + std::to_string(sizeBytes) };
    }
    bindResult(reg, operand);
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName, int accessSizeBytes) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    // result holds the destination address; operand is the value to store.
    Register& resultRegister = residesInMemory(result) ? assignRegisterTo(result) : result.getAssignedRegister();
    const int words = wordCount(operand);
    if (words <= 1) {
        Register& operandRegister = residesInMemory(operand)
                ? assignRegisterExcluding(operand, resultRegister)
                : operand.getAssignedRegister();
        // If operand landed in the same reg as the address, spill address first.
        if (&operandRegister == &resultRegister) {
            Register& addr = get64BitRegisterExcluding(operandRegister);
            assembly << instructionSet->mov(resultRegister, addr);
            if (accessSizeBytes == 1) {
                assembly << ("mov byte " + memoryAt(addr, 0) + ", " + reg8Name(operandRegister));
            } else if (accessSizeBytes == 2) {
                assembly << ("mov word " + memoryAt(addr, 0) + ", " + reg16Name(operandRegister));
            } else if (accessSizeBytes == 4) {
                assembly << ("mov dword " + memoryAt(addr, 0) + ", " + reg32Name(operandRegister));
            } else {
                assembly << instructionSet->mov(operandRegister, MemoryOperand::at(addr, 0));
            }
        } else if (accessSizeBytes == 1) {
            assembly << ("mov byte " + memoryAt(resultRegister, 0) + ", " + reg8Name(operandRegister));
        } else if (accessSizeBytes == 2) {
            assembly << ("mov word " + memoryAt(resultRegister, 0) + ", " + reg16Name(operandRegister));
        } else if (accessSizeBytes == 4) {
            assembly << ("mov dword " + memoryAt(resultRegister, 0) + ", " + reg32Name(operandRegister));
        } else {
            assembly << instructionSet->mov(operandRegister, MemoryOperand::at(resultRegister, 0));
        }
        return;
    }
    // Multi-word (struct) store through pointer: copy each word to [addr+off].
    // Use exact object size so a 36-byte struct does not write a 5th full qword
    // into the next array element (git object_id / khash keys[]).
    const int totalSize = operand.getSizeInBytes();
    for (int w = 0; w < words; ++w) {
        // Re-load address if it may have been spilled by loadWord register pressure.
        Register& addr = residesInMemory(result) ? assignRegisterTo(result) : result.getAssignedRegister();
        Register& tmp = get64BitRegisterExcluding(addr);
        loadWord(operand, w, tmp);
        // loadWord may have stolen addr's register; re-resolve address.
        Register& addr2 = residesInMemory(result) ? assignRegisterExcluding(result, tmp) : result.getAssignedRegister();
        const int byteOff = w * MACHINE_WORD_SIZE;
        int storeBytes = MACHINE_WORD_SIZE;
        if (totalSize > 0) {
            const int remaining = totalSize - byteOff;
            if (remaining <= 0) {
                break;
            }
            if (remaining < MACHINE_WORD_SIZE) {
                storeBytes = remaining;
            }
        }
        if (&addr2 == &tmp) {
            Register& spare = get64BitRegisterExcluding(tmp);
            // Should not happen with assignRegisterExcluding; defensive.
            assembly << instructionSet->mov(addr2, spare);
            storeBytesAt(tmp, spare, byteOff, storeBytes);
        } else {
            storeBytesAt(tmp, addr2, byteOff, storeBytes);
        }
    }
}

void StackMachine::storeInMemory(Value& symbol) {
    if (!symbol.isStored()) {
        storeRegisterValue(symbol.getAssignedRegister());
    }
}

void StackMachine::loadWord(Value& symbol, int wordIndex, Register& dest) {
    // Single-word values may be register-resident; multi-word objects always live in memory.
    if (wordIndex == 0 && wordCount(symbol) == 1 && !residesInMemory(symbol)) {
        if (&dest != &symbol.getAssignedRegister()) {
            assembly << instructionSet->mov(symbol.getAssignedRegister(), dest);
        }
        return;
    }
    Address home = addressOf(symbol);
    const int byteOff = object_abi::wordByteOffset(wordIndex);
    if (home.isGlobal()) {
        Register& addr = get64BitRegisterExcluding(dest);
        assembly << instructionSet->lea(memoryOperand(home), addr);
        if (byteOff) {
            assembly << instructionSet->add(addr, byteOff);
        }
        assembly << instructionSet->mov(MemoryOperand::at(addr, 0), dest);
    } else {
        Address wordHome = Address::frame(home.frameBase(), home.offsetBytes() + byteOff, MACHINE_WORD_SIZE);
        assembly << instructionSet->mov(memoryOperand(wordHome), dest);
    }
}

void StackMachine::storeBytesAt(Register& source, Register& addr, int offset, int nbytes) {
    if (nbytes <= 0) {
        return;
    }
    if (nbytes >= 8) {
        assembly << instructionSet->mov(source, MemoryOperand::at(addr, offset));
        return;
    }
    if (nbytes >= 4) {
        assembly << ("mov dword " + memoryAt(addr, offset) + ", " + reg32Name(source));
        if (nbytes > 4) {
            // Remaining 1-3 bytes live in bits 32.. of the word.
            std::vector<Register*> exclude { &source, &addr };
            Register& hi = get64BitRegisterExcluding(exclude);
            assembly << instructionSet->mov(source, hi);
            assembly << ("shr " + hi.getName() + ", 32");
            storeBytesAt(hi, addr, offset + 4, nbytes - 4);
        }
        return;
    }
    if (nbytes >= 2) {
        assembly << ("mov word " + memoryAt(addr, offset) + ", " + reg16Name(source));
        if (nbytes > 2) {
            std::vector<Register*> exclude { &source, &addr };
            Register& hi = get64BitRegisterExcluding(exclude);
            assembly << instructionSet->mov(source, hi);
            assembly << ("shr " + hi.getName() + ", 16");
            assembly << ("mov byte " + memoryAt(addr, offset + 2) + ", " + reg8Name(hi));
        }
        return;
    }
    assembly << ("mov byte " + memoryAt(addr, offset) + ", " + reg8Name(source));
}

void StackMachine::storeWord(Register& source, Value& symbol, int wordIndex) {
    if (wordIndex == 0 && wordCount(symbol) == 1 && !residesInMemory(symbol)) {
        if (&source != &symbol.getAssignedRegister()) {
            assembly << instructionSet->mov(source, symbol.getAssignedRegister());
        }
        return;
    }
    Address home = addressOf(symbol);
    const int byteOff = object_abi::wordByteOffset(wordIndex);
    // Cap store to remaining object bytes so packed array elements (e.g. 36-byte
    // object_id) are not written as a full last qword.
    int storeBytes = MACHINE_WORD_SIZE;
    const int totalSize = symbol.getSizeInBytes();
    if (totalSize > 0) {
        const int remaining = totalSize - byteOff;
        if (remaining <= 0) {
            return;
        }
        if (remaining < MACHINE_WORD_SIZE) {
            storeBytes = remaining;
        }
    }
    if (home.isGlobal()) {
        Register& addr = get64BitRegisterExcluding(source);
        assembly << instructionSet->lea(memoryOperand(home), addr);
        if (byteOff) {
            assembly << instructionSet->add(addr, byteOff);
        }
        storeBytesAt(source, addr, 0, storeBytes);
    } else if (storeBytes == MACHINE_WORD_SIZE) {
        Address wordHome = Address::frame(home.frameBase(), home.offsetBytes() + byteOff, MACHINE_WORD_SIZE);
        assembly << instructionSet->mov(source, memoryOperand(wordHome));
    } else {
        // Partial last word of a multi-word local/temp: LEA then sized store.
        Address wordHome = Address::frame(home.frameBase(), home.offsetBytes() + byteOff, storeBytes);
        Register& addr = get64BitRegisterExcluding(source);
        assembly << instructionSet->lea(memoryOperand(wordHome), addr);
        storeBytesAt(source, addr, 0, storeBytes);
    }
}

void StackMachine::copyWords(Value& source, Value& destination) {
    // Copy only destination's exact size (source may be larger padding-wise).
    // wordCount still drives the loop, but storeWord caps the last partial word.
    const int words = std::max(wordCount(source), wordCount(destination));
    Register& reg = get64BitRegister();
    for (int w = 0; w < words; ++w) {
        loadWord(source, w, reg);
        storeWord(reg, destination, w);
    }
}

void StackMachine::fieldAddress(std::string baseName, int offsetBytes, std::string resultName, bool baseIsPointer) {
    auto& base = resolve(baseName);
    Register& addrReg = get64BitRegister();
    if (baseIsPointer) {
        // Arrow: base holds a pointer value (object address).
        if (residesInMemory(base)) {
            emitLoad(base, addrReg);
        } else {
            assembly << instructionSet->mov(base.getAssignedRegister(), addrReg);
        }
    } else {
        // Dot: base is the object; take its address. Spill first so register-
        // passed by-value structs (single eightbyte in an arg reg) land in the
        // frame home before we form &object+offset (git-style Point {int,int}).
        storeInMemory(base);
        assembly << instructionSet->lea(memoryOperand(base), addrReg);
    }
    if (offsetBytes) {
        assembly << instructionSet->add(addrReg, offsetBytes);
    }
    bindResult(addrReg, resolve(resultName));
}

void StackMachine::indexAddress(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
        bool baseIsArray) {
    auto& base = resolve(baseName);
    auto& index = resolve(indexName);

    // One-operand imul writes RDX:RAX. Scale the index first, then form the base address in a
    // register other than RAX/RDX so the product high half cannot clobber it.
    Register& mulReg = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(mulReg);
    storeRegisterValue(rdx);

    auto pickRegExcluding = [this](Register* a, Register* b) -> Register* {
        for (auto& reg : registers->getGeneralPurposeRegisters()) {
            if (reg != a && reg != b) {
                storeRegisterValue(*reg);
                return reg;
            }
        }
        throw std::runtime_error { "unable to get free register for index addressing" };
    };

    if (elementSizeBytes != 1) {
        assignRegisterToSymbol(mulReg, index);
        Register* sizeReg = pickRegExcluding(&mulReg, &rdx);
        assembly << instructionSet->mov(std::to_string(elementSizeBytes), *sizeReg);
        assembly << instructionSet->imul(*sizeReg);
        // scaled index is in mulReg (rax); rdx clobbered
    }

    Register* addr = pickRegExcluding(&mulReg, &rdx);
    if (baseIsArray) {
        storeInMemory(base);
        assembly << instructionSet->lea(memoryOperand(base), *addr);
    } else if (residesInMemory(base)) {
        emitLoad(base, *addr);
    } else {
        assembly << instructionSet->mov(base.getAssignedRegister(), *addr);
    }

    if (elementSizeBytes == 1) {
        // Same sub-word rule as add(): do not add a dirty 4-byte stack slot as qword.
        if (residesInMemory(index)
                && index.getType() == Type::INTEGRAL
                && index.getSizeInBytes() > 0
                && index.getSizeInBytes() < MACHINE_WORD_SIZE) {
            Register& indexReg = get64BitRegisterExcluding({ addr, &mulReg, &rdx });
            emitLoad(index, indexReg);
            assembly << instructionSet->add(indexReg, *addr);
        } else if (residesInMemory(index)) {
            assembly << instructionSet->add(memoryOperand(index), *addr);
        } else {
            assembly << instructionSet->add(index.getAssignedRegister(), *addr);
        }
    } else {
        assembly << instructionSet->add(mulReg, *addr);
    }
    bindResult(*addr, resolve(resultName));
}


} // namespace codegen
