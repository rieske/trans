#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "ObjectAbi.h"

namespace codegen {

StackMachine::StackMachine(std::ostream *ostream, std::unique_ptr<InstructionSet> instructionSet, std::unique_ptr<Amd64Registers> registers) :
        assembly{ostream},
        instructionSet{std::move(instructionSet)},
        registers{std::move(registers)} {}

void StackMachine::spillGeneralPurposeRegisters() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::spillCallerSavedRegisters() {
    for (auto& reg : registers->getCallerSavedRegisters()) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::emitLoad(Value& symbol, Register& dest) {
    assembly << instructionSet->mov(memoryOperand(symbol), dest);
    // Locals pad sub-word C types to 8-byte stack slots; stores through T* write
    // only the type width (e.g. int* -> dword). Narrow the loaded value so dirty
    // high bits do not poison compares or calls (git: peel_object_ext writes
    // *typep as dword, then type != OBJ_COMMIT / type_name(type)).
    if (symbol.getType() == Type::INTEGRAL) {
        const int size = symbol.getSizeInBytes();
        if (size == 1) {
            if (symbol.isSigned()) {
                assembly << ("movsx " + dest.getName() + ", " + reg8Name(dest));
            } else {
                assembly << ("movzx " + dest.getName() + ", " + reg8Name(dest));
            }
        } else if (size == 2) {
            if (symbol.isSigned()) {
                assembly << ("movsx " + dest.getName() + ", " + reg16Name(dest));
            } else {
                assembly << ("movzx " + dest.getName() + ", " + reg16Name(dest));
            }
        } else if (size == 4) {
            if (symbol.isSigned()) {
                assembly << ("movsxd " + dest.getName() + ", " + reg32Name(dest));
            } else {
                // 32-bit mov zero-extends into the full 64-bit register.
                assembly << ("mov " + reg32Name(dest) + ", " + reg32Name(dest));
            }
        }
    }
}

void StackMachine::loadWithoutBinding(Value& symbol, Register& dest) {
    emitLoad(symbol, dest);
}

void StackMachine::emitStore(Register& source, Value& symbol) {
    assembly << instructionSet->mov(source, memoryOperand(symbol));
}

// Bind a freshly computed result to its destination symbol. Global homes are Address-only:
// commit to memory; never attach a register to the global Value (loads use scratch only).
// Locals/temps use register residence and lazy write-back.
void StackMachine::bindResult(Register& reg, Value& result) {
    if (addressOf(result).isGlobal()) {
        emitStore(reg, result);
        assert(result.isStored() && "global Value must not be register-linked");
        return;
    }
    reg.assign(&result);
}

void StackMachine::assignRegisterToSymbol(Register& reg, Value& symbol) {
    // Always load without binding when the symbol is memory-resident (includes all globals:
    // their homes are Address-only; never reg.assign the global Value).
    if (residesInMemory(symbol)) {
        storeRegisterValue(reg);
        loadWithoutBinding(symbol, reg);
    } else if (&reg != &symbol.getAssignedRegister()) {
        storeRegisterValue(reg);
        Register& valueRegister = symbol.getAssignedRegister();
        storeRegisterValue(valueRegister);
        assembly << instructionSet->mov(valueRegister, reg);
    }
}

void StackMachine::storeRegisterValue(Register& reg) {
    if (!reg.containsUnstoredValue()) {
        return;
    }
    Value* value = reg.getValue();
    const int lastUse = value->getLastUseOrdinal();
    // Dead expression temp: discard without writing a possibly-reused stack slot.
    if (lastUse >= 0 && lastUse < instructionOrdinal) {
        reg.free();
        return;
    }
    emitStore(reg, *value);
    reg.free();
}

void StackMachine::emptyGeneralPurposeRegisters() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        reg->free();
    }
}

void StackMachine::pushCalleeSavedRegisters() { pushRegisters(registers->getCalleeSavedRegisters(), calleeSavedRegisters); }

void StackMachine::popCalleeSavedRegisters() { popRegisters(calleeSavedRegisters); }

void StackMachine::pushRegisters(std::vector<Register*> source, std::vector<Register*>& destination) {
    for (auto& reg : source) {
        pushRegister(*reg, destination);
    }
}

void StackMachine::popRegisters(std::vector<Register*> registers) {
    for (auto& reg : registers) {
        assembly << instructionSet->pop(*reg);
    }
}

void StackMachine::pushRegister(Register& reg, std::vector<Register*>& registers) {
    registers.insert(registers.begin(), &reg);
    assembly << instructionSet->push(reg);
}

Address StackMachine::spillSlotAddress(const Value& symbol) const {
    int offset = symbol.getIndex() * MACHINE_WORD_SIZE
            + static_cast<int>(calleeSavedRegisters.size()) * MACHINE_WORD_SIZE;
    return Address::frame(FrameBase::StackPointer, offset, symbol.getSizeInBytes());
}

bool StackMachine::residesInMemory(const Value& symbol) const {
    return addressOf(symbol).isGlobal() || symbol.isStored();
}

Address StackMachine::addressOf(const Value& symbol) const {
    const std::string& name = symbol.getName();
    auto frame = frameHomes.find(name);
    if (frame != frameHomes.end()) {
        return frame->second;
    }
    auto global = globalHomes.find(name);
    if (global != globalHomes.end()) {
        return global->second;
    }
    // No registered home: a temporary. Its spill slot is derived from Value::index, which the
    // code generator must keep consistent with the value's position among the frame's locals.
    return spillSlotAddress(symbol);
}

MemoryOperand StackMachine::memoryOperand(const Address& address) const {
    if (address.isGlobal()) {
        return MemoryOperand::global(address.label());
    }
    const Register& base = address.frameBase() == FrameBase::BasePointer ?
            registers->getBasePointer() : registers->getStackPointer();
    return MemoryOperand::at(base, address.offsetBytes());
}

MemoryOperand StackMachine::memoryOperand(const Value& symbol) const {
    return memoryOperand(addressOf(symbol));
}

Register& StackMachine::get64BitRegister() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    Register& reg = **registers->getGeneralPurposeRegisters().begin();
    storeRegisterValue(reg);
    return reg;
}

Register& StackMachine::get64BitRegisterExcluding(Register& registerToExclude) {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &registerToExclude && !reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &registerToExclude) {
            storeRegisterValue(*reg);
            return *reg;
        }
    }
    throw std::runtime_error{"unable to get a free register"};
}

Register& StackMachine::get64BitRegisterExcluding(const std::vector<Register*>& registersToExclude) {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!registerInList(reg, registersToExclude) && !reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!registerInList(reg, registersToExclude)) {
            storeRegisterValue(*reg);
            return *reg;
        }
    }
    throw std::runtime_error{"unable to get a free register"};
}

Register& StackMachine::get64BitRegisterExcluding(Register& registerToExclude,
        const std::vector<Register*>& registersToExclude) {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &registerToExclude && !registerInList(reg, registersToExclude)
                && !reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &registerToExclude && !registerInList(reg, registersToExclude)) {
            storeRegisterValue(*reg);
            return *reg;
        }
    }
    throw std::runtime_error{"unable to get a free register"};
}

Register& StackMachine::getCounterRegister() {
    Register& counter = registers->getCounterRegister();
    storeRegisterValue(counter);
    return counter;
}

Register& StackMachine::assignRegisterTo(Value& symbol) {
    Register& reg = get64BitRegister();
    loadWithoutBinding(symbol, reg);
    // Bind only non-global homes; globals stay Address-only (scratch in reg).
    if (!addressOf(symbol).isGlobal()) {
        reg.assign(&symbol);
    }
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value& symbol, Register& registerToExclude) {
    Register& reg = get64BitRegisterExcluding(registerToExclude);
    loadWithoutBinding(symbol, reg);
    if (!addressOf(symbol).isGlobal()) {
        reg.assign(&symbol);
    }
    return reg;
}

Value& StackMachine::resolve(const std::string& name) {
    auto local = scopeValues.find(name);
    if (local != scopeValues.end()) {
        return local->second;
    }
    auto global = globals.find(name);
    if (global != globals.end()) {
        return global->second;
    }
    throw std::runtime_error {
            "codegen: no storage for symbol `" + name + "` (function designator or missing global?)" };
}





int StackMachine::wordCount(const Value& symbol) const {
    return object_abi::valueWords(symbol.getSizeInBytes());
}

} // namespace codegen
