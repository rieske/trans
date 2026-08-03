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

void StackMachine::generatePreamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) {
    // Pre-register all procedures so earlier TUs can call later ones with direct call (not PLT).
    for (const auto& name : definedFunctions) {
        if (!name.empty() && name[0] == '.') {
            definedProcedures.insert(name.substr(1));
        } else {
            definedProcedures.insert(name);
        }
    }
    assembly.raw(instructionSet->preamble(constants, globalVariables, externalFunctions, definedFunctions));
    for (const auto& global : globalVariables) {
        globalHomes.emplace(global.name, Address::globalLabel(global.name, global.sizeInBytes));
        // resolve() shell only; home is globalHomes, never register-cached.
        globals.emplace(global.name, global.toValue());
    }
}

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments,
        bool variadic, bool memoryReturn) {
    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    sretSymbolName.clear();
    procedureIsVariadic = variadic;
    lastNamedFormalName = arguments.empty() ? std::string {} : arguments.back().getName();
    vaStartState = {};
    instructionOrdinal = 0;
    definedProcedures.insert(procedureName);
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto& value : values) {
        scopeValues.insert({value.getName(), value});
    }

    // Variadic callees: GP/XMM save area so va_arg walks register-passed extras.
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();

    SysVArgCounts argCounts;
    // Floating parameters that arrive in xmmN; copied after frame setup (non-variadic).
    std::vector<std::pair<std::string, std::size_t>> floatingRegArgs;
    // Next free spill-slot word index. Must be max(index+words) over locals, not
    // scopeValues.size() (entry count): a multi-word local occupies several words
    // under one map entry, so using size() overlaps arg homes with the local
    // (git strbuf_init: blank is 3 words; hint's home collided and was clobbered).
    int localIndex = 0;
    for (const auto& entry : scopeValues) {
        int words = type::object_abi::valueWords(entry.second.getSizeInBytes());
        localIndex = std::max(localIndex, entry.second.getIndex() + words);
    }
    int argumentWordIndex{0};

    // System V memory return: first integer arg register holds the hidden pointer.
    // Product: sret only when memoryReturn && !variadic (see variadicMemoryReturnSkipsSret).
    if (memoryReturn && !variadic) {
        sretSymbolName = type::object_abi::SRET_SYMBOL_NAME;
        Value sret { sretSymbolName, localIndex, ValueKind::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ sretSymbolName, sret });
        integerArgRegs[0]->assign(&resolve(sretSymbolName));
        argCounts.integerRegs = 1;
        ++localIndex;
    }

    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    int vaSaveBaseIndex = variadic ? localIndex : -1;
    std::vector<std::string> vaGpHome(SYSV_INTEGER_ARG_REGS);
    std::vector<std::string> vaXmmHome(SYSV_SSE_ARG_REGS);

    for (auto& argument : arguments) {
        const int words = type::object_abi::valueWords(argument.getSizeInBytes());
        const SysVArgPlacement place = takeSysVArgSlot(argument, argCounts, SYSV_INTEGER_ARG_REGS);
        vaStartState.lastNamedOnStack = place.slot == SysVArgSlot::Stack;
        switch (place.slot) {
        case SysVArgSlot::SseReg: {
            int homeIndex;
            if (variadic) {
                homeIndex = vaSaveBaseIndex + vaGpSlots
                        + static_cast<int>(place.index) * vaXmmWordsEach;
                vaXmmHome[place.index] = argument.getName();
            } else {
                homeIndex = localIndex;
                ++localIndex;
                floatingRegArgs.push_back({ argument.getName(), place.index });
            }
            Value floatArgument{argument.getName(), homeIndex,
                    argument.getValueKind(), argument.getSizeInBytes(), argument.isSigned()};
            scopeValues.insert({argument.getName(), floatArgument});
            break;
        }
        case SysVArgSlot::IntegerReg: {
            // Preserve isSigned so unsigned int params zero-extend on load
            // (git sha256 ror/gamma on 0x80000000; default true breaks >>).
            if (variadic) {
                Value regSaveArg{argument.getName(), vaSaveBaseIndex + static_cast<int>(place.index),
                        argument.getValueKind(), argument.getSizeInBytes(), argument.isSigned()};
                scopeValues.insert({argument.getName(), regSaveArg});
                vaGpHome[place.index] = argument.getName();
            } else {
                Value registerArgument{argument.getName(), localIndex,
                        argument.getValueKind(), argument.getSizeInBytes(), argument.isSigned()};
                scopeValues.insert({argument.getName(), registerArgument});
                integerArgRegs[place.index]->assign(&resolve(argument.getName()));
                ++localIndex;
            }
            break;
        }
        case SysVArgSlot::Stack: {
            Value stackArgument{argument.getName(), argumentWordIndex, argument.getValueKind(),
                    argument.getSizeInBytes(), argument.isSigned()};
            scopeValues.insert({argument.getName(), stackArgument});
            registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                    (argumentWordIndex + 2) * MACHINE_WORD_SIZE, argument.getSizeInBytes()));
            argumentWordIndex += words;
            break;
        }
        }
    }

    vaStartState.gpOffset = sysvNamedGpOffset(argCounts);
    vaStartState.fpOffset = sysvNamedFpOffset(argCounts);

    if (variadic) {
        for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
            if (!vaGpHome[i].empty()) {
                continue;
            }
            std::string slotName = "__va_reg_" + std::to_string(i);
            Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), ValueKind::INTEGRAL,
                    MACHINE_WORD_SIZE };
            scopeValues.insert({ slotName, slot });
            vaGpHome[i] = slotName;
        }
        // Unused XMM save slots (16-byte stride). Named SSE formals already occupy theirs.
        for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
            if (!vaXmmHome[i].empty()) {
                continue;
            }
            std::string slotName = "__va_xmm_" + std::to_string(i);
            Value slot { slotName, vaSaveBaseIndex + vaGpSlots + static_cast<int>(i) * vaXmmWordsEach,
                    ValueKind::INTEGRAL, MACHINE_WORD_SIZE * vaXmmWordsEach };
            scopeValues.insert({ slotName, slot });
            vaXmmHome[i] = slotName;
        }
    }

    int savedRegistersStack = registers->getCalleeSavedRegisters().size() * MACHINE_WORD_SIZE;
    int maxWordEnd = 0;
    for (const auto& entry : scopeValues) {
        if (frameHomes.count(entry.first)) {
            continue;
        }
        int words = type::object_abi::valueWords(entry.second.getSizeInBytes());
        maxWordEnd = std::max(maxWordEnd, entry.second.getIndex() + words);
    }
    localVariableStackSize = maxWordEnd * MACHINE_WORD_SIZE;
    int stackSize = savedRegistersStack + localVariableStackSize;
    if (stackSize % STACK_ALIGNMENT) {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize + MACHINE_WORD_SIZE);
    } else {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize);
    }

    pushCalleeSavedRegisters();
    for (const auto& entry : scopeValues) {
        if (frameHomes.count(entry.first)) {
            continue;
        }
        registerFrameHome(entry.first, spillSlotAddress(entry.second));
    }

    // Copy incoming floating register args (xmmN) into their spill homes.
    for (const auto& item : floatingRegArgs) {
        auto& home = resolve(item.first);
        Register& tmp = registers->getRetrievalRegister();
        xmmToGpr(static_cast<int>(item.second), tmp, sseWidth(home));
        emitStore(tmp, home);
    }

    if (variadic) {
        for (std::size_t i = 0; i < SYSV_INTEGER_ARG_REGS; ++i) {
            auto& home = resolve(vaGpHome[i]);
            Address addr = addressOf(home);
            assembly << instructionSet->mov(*integerArgRegs[i], memoryOperand(addr));
        }
        // Save xmm0..xmm7 into the reg_save_area at 16-byte strides (SysV).
        for (std::size_t i = 0; i < SYSV_SSE_ARG_REGS; ++i) {
            Address addr = addressOf(resolve(vaXmmHome[i]));
            const Register& base = addr.frameBase() == FrameBase::BasePointer
                    ? registers->getBasePointer()
                    : registers->getStackPointer();
            // Low 64 bits of xmm (enough for double) into the save slot.
            assembly << instructionSet->sseXmmToMem(static_cast<int>(i),
                    MemoryOperand::at(base, addr.offsetBytes()));
        }
        // Push save-area pointers onto the thread-local stack (runtime/va_tls.c).
        // A single TLS slot is wrong for nested variadic calls (outer prologue,
        // then inner strvec_pushf, then outer va_start) and for concurrent
        // threads (git index-pack: format_object_header / xsnprintf).
        auto& scratch = registers->getRetrievalRegister();
        Address saveBase = addressOf(resolve(vaGpHome[0]));
        assembly << instructionSet->lea(memoryOperand(saveBase), scratch);
        assembly << instructionSet->mov(scratch, *integerArgRegs[0]);
        // First stack argument lives at [rbp+16].
        Address overflow = Address::frame(FrameBase::BasePointer, 2 * MACHINE_WORD_SIZE, MACHINE_WORD_SIZE);
        assembly << instructionSet->lea(memoryOperand(overflow), scratch);
        assembly << instructionSet->mov(scratch, *integerArgRegs[1]);
        emitNamedCall("__trans_va_set_areas");
    }
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
    frameHomes.clear();
    calleeSavedRegisters.clear();
    sretSymbolName.clear();
    procedureIsVariadic = false;
    lastNamedFormalName.clear();
    vaStartState = {};
    instructionOrdinal = 0;
}

void StackMachine::registerFrameHome(const std::string& name, Address address) {
    [[maybe_unused]] auto inserted = frameHomes.emplace(name, std::move(address)).second;
    assert(inserted && "duplicate frame home registration");
}

void StackMachine::setScope(std::vector<Value> variables) {
    for (auto& var : variables) {
        scopeValues.insert({var.getName(), var});
        registerFrameHome(var.getName(), spillSlotAddress(var));
    }
}


} // namespace codegen
