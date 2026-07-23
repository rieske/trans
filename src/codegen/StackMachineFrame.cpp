#include "StackMachine.h"
#include "StackMachineInternal.h"

#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <vector>

#include "InstructionSet.h"
#include "ObjectAbi.h"

namespace codegen {

void StackMachine::generatePreamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) {
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
    instructionOrdinal = 0;
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto& value : values) {
        scopeValues.insert({value.getName(), value});
    }

    // Variadic callees: give every integer arg register a memory home in a contiguous
    // save area so va_start(ap, last) as &last+8 walks register-passed variadic args.
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t regArgCapacity = integerArgRegs.size(); // 6 on SysV AMD64

    std::size_t integerArgumentRegisterIndex{0};
    // Floating parameters that arrive in xmm0..xmm7 (non-variadic); stored after frame setup.
    std::vector<std::string> floatingRegArgNames;
    // Next free spill-slot word index. Must be max(index+words) over locals, not
    // scopeValues.size() (entry count): a multi-word local occupies several words
    // under one map entry, so using size() overlaps arg homes with the local
    // (git strbuf_init: blank is 3 words; hint's home collided and was clobbered).
    int localIndex = 0;
    for (const auto& entry : scopeValues) {
        int words = object_abi::valueWords(entry.second.getSizeInBytes());
        localIndex = std::max(localIndex, entry.second.getIndex() + words);
    }
    int argumentWordIndex{0};

    // System V memory return: first integer arg register holds the hidden pointer.
    if (memoryReturn && !variadic) {
        sretSymbolName = object_abi::SRET_SYMBOL_NAME;
        Value sret { sretSymbolName, localIndex, Type::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ sretSymbolName, sret });
        integerArgRegs[0]->assign(&resolve(sretSymbolName));
        integerArgumentRegisterIndex = 1;
        ++localIndex;
    }

    // SysV AMD64 reg_save_area: 6 GP qwords (0..47) + 8 XMM slots of 16 bytes (48..175).
    constexpr int vaGpSlots = 6;
    constexpr int vaXmmCount = 8;
    constexpr int vaXmmWordsEach = 2; // 16-byte stride
    int vaSaveBaseIndex = -1;

    if (variadic) {
        int maxLocalWordEnd = 0;
        for (const auto& entry : scopeValues) {
            int words = object_abi::valueWords(entry.second.getSizeInBytes());
            maxLocalWordEnd = std::max(maxLocalWordEnd, entry.second.getIndex() + words);
        }
        vaSaveBaseIndex = maxLocalWordEnd;
        for (std::size_t i = 0; i < arguments.size(); ++i) {
            auto& argument = arguments[i];
            const int words = object_abi::valueWords(argument.getSizeInBytes());
            if (words == 1 && i < regArgCapacity) {
                // Named register args live inside the reg_save_area so va_start can
                // compute gp_offset from &last relative to the save base.
                Value regSaveArg { argument.getName(), vaSaveBaseIndex + static_cast<int>(i),
                        argument.getType(), argument.getSizeInBytes(), argument.isSigned() };
                scopeValues.insert({ argument.getName(), regSaveArg });
            } else if (words == 1) {
                const int stackIndex = static_cast<int>(i - regArgCapacity);
                Value stackArgument { argument.getName(), stackIndex, argument.getType(),
                        argument.getSizeInBytes(), argument.isSigned() };
                scopeValues.insert({ argument.getName(), stackArgument });
                registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                        (stackIndex + 2) * MACHINE_WORD_SIZE, argument.getSizeInBytes()));
            } else {
                Value stackArgument { argument.getName(), argumentWordIndex, argument.getType(),
                        argument.getSizeInBytes(), argument.isSigned() };
                scopeValues.insert({ argument.getName(), stackArgument });
                registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                        (argumentWordIndex + 2) * MACHINE_WORD_SIZE, argument.getSizeInBytes()));
                argumentWordIndex += words;
            }
        }
        for (std::size_t i = 0; i < regArgCapacity; ++i) {
            std::string slotName = "__va_reg_" + std::to_string(i);
            if (scopeValues.count(slotName)) {
                continue;
            }
            bool namedOccupies = false;
            for (std::size_t a = 0; a < arguments.size() && a < regArgCapacity; ++a) {
                if (a == i) {
                    namedOccupies = true;
                    break;
                }
            }
            if (namedOccupies) {
                continue;
            }
            Value slot { slotName, vaSaveBaseIndex + static_cast<int>(i), Type::INTEGRAL, MACHINE_WORD_SIZE };
            scopeValues.insert({ slotName, slot });
        }
        // XMM save slots (16-byte stride). Overflow args stay on the caller's stack
        // at [rbp+16]; libc and va_arg walk them via overflow_arg_area.
        for (int i = 0; i < vaXmmCount; ++i) {
            std::string slotName = "__va_xmm_" + std::to_string(i);
            Value slot { slotName, vaSaveBaseIndex + vaGpSlots + i * vaXmmWordsEach,
                    Type::INTEGRAL, MACHINE_WORD_SIZE * vaXmmWordsEach };
            scopeValues.insert({ slotName, slot });
        }
    } else {
        for (auto& argument : arguments) {
            const int words = object_abi::valueWords(argument.getSizeInBytes());
            if (words == 1 && argument.getType() == Type::FLOATING && floatingRegArgNames.size() < 8) {
                // Double/float arrive in xmmN; give a spill home and copy after frame setup.
                Value floatArgument{argument.getName(), static_cast<int>(localIndex),
                        argument.getType(), argument.getSizeInBytes(), argument.isSigned()};
                scopeValues.insert({argument.getName(), floatArgument});
                floatingRegArgNames.push_back(argument.getName());
                ++localIndex;
            } else if (words == 1 && integerArgumentRegisterIndex < regArgCapacity) {
                // Preserve isSigned so unsigned int params zero-extend on load
                // (git sha256 ror/gamma on 0x80000000; default true breaks >>).
                Value registerArgument{argument.getName(), static_cast<int>(localIndex),
                        argument.getType(), argument.getSizeInBytes(), argument.isSigned()};
                scopeValues.insert({argument.getName(), registerArgument});
                integerArgRegs[integerArgumentRegisterIndex]->assign(&resolve(argument.getName()));
                ++integerArgumentRegisterIndex;
                ++localIndex;
            } else {
                Value stackArgument{argument.getName(), argumentWordIndex, argument.getType(),
                        argument.getSizeInBytes(), argument.isSigned()};
                scopeValues.insert({argument.getName(), stackArgument});
                registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                        (argumentWordIndex + 2) * MACHINE_WORD_SIZE, argument.getSizeInBytes()));
                argumentWordIndex += words;
            }
        }
    }

    int savedRegistersStack = registers->getCalleeSavedRegisters().size() * MACHINE_WORD_SIZE;
    int maxWordEnd = 0;
    for (const auto& entry : scopeValues) {
        if (frameHomes.count(entry.first)) {
            continue;
        }
        int words = object_abi::valueWords(entry.second.getSizeInBytes());
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

    // Copy incoming floating register args (xmm0..) into their spill homes.
    static const char* const xmmIncoming[] = {
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    };
    for (std::size_t i = 0; i < floatingRegArgNames.size() && i < 8; ++i) {
        auto& home = resolve(floatingRegArgNames[i]);
        Register& tmp = registers->getRetrievalRegister();
        assembly << (std::string("movq ") + tmp.getName() + ", " + xmmIncoming[i]);
        emitStore(tmp, home);
    }

    if (variadic) {
        for (std::size_t i = 0; i < regArgCapacity; ++i) {
            std::string homeName;
            if (i < arguments.size()) {
                const int words = object_abi::valueWords(arguments[i].getSizeInBytes());
                if (words == 1) {
                    homeName = arguments[i].getName();
                }
            }
            if (homeName.empty()) {
                homeName = "__va_reg_" + std::to_string(i);
            }
            if (!scopeValues.count(homeName)) {
                continue;
            }
            auto& home = resolve(homeName);
            Address addr = addressOf(home);
            assembly << instructionSet->mov(*integerArgRegs[i], memoryOperand(addr));
        }
        // Save xmm0..xmm7 into the reg_save_area at 16-byte strides (SysV).
        for (int i = 0; i < vaXmmCount; ++i) {
            std::string slotName = "__va_xmm_" + std::to_string(i);
            if (!scopeValues.count(slotName)) {
                continue;
            }
            Address addr = addressOf(resolve(slotName));
            const Register& base = addr.frameBase() == FrameBase::BasePointer
                    ? registers->getBasePointer()
                    : registers->getStackPointer();
            // NASM Intel: movq [mem], xmmN stores the low 64 bits (enough for double).
            assembly << (std::string("movq ") + memoryAt(base, addr.offsetBytes())
                    + ", xmm" + std::to_string(i));
        }
        // Push save-area pointers onto the thread-local stack (runtime/va_tls.c).
        // A single TLS slot is wrong for nested variadic calls (outer prologue,
        // then inner strvec_pushf, then outer va_start) and for concurrent
        // threads (git index-pack: format_object_header / xsnprintf).
        auto& scratch = registers->getRetrievalRegister();
        const auto& argRegs = registers->getIntegerArgumentRegisters();
        for (const auto& entry : scopeValues) {
            if (entry.second.getIndex() == vaSaveBaseIndex) {
                Address saveBase = addressOf(resolve(entry.first));
                assembly << instructionSet->lea(memoryOperand(saveBase), scratch);
                assembly << instructionSet->mov(scratch, *argRegs[0]);
                break;
            }
        }
        // First stack argument lives at [rbp+16].
        Address overflow = Address::frame(FrameBase::BasePointer, 2 * MACHINE_WORD_SIZE, MACHINE_WORD_SIZE);
        assembly << instructionSet->lea(memoryOperand(overflow), scratch);
        assembly << instructionSet->mov(scratch, *argRegs[1]);
        assembly << instructionSet->call("__trans_va_set_areas");
    }
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
    frameHomes.clear();
    calleeSavedRegisters.clear();
    sretSymbolName.clear();
    procedureIsVariadic = false;
    instructionOrdinal = 0;
}

void StackMachine::registerFrameHome(const std::string& name, Address address) {
    auto inserted = frameHomes.emplace(name, std::move(address)).second;
    assert(inserted && "duplicate frame home registration");
}

void StackMachine::setScope(std::vector<Value> variables) {
    for (auto& var : variables) {
        scopeValues.insert({var.getName(), var});
        registerFrameHome(var.getName(), spillSlotAddress(var));
    }
}


} // namespace codegen
