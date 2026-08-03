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
        definedProcedures.insert(name);
    }
    assembly.raw(instructionSet->preamble(constants, globalVariables, externalFunctions));
    for (const auto& global : globalVariables) {
        globalHomes.emplace(global.name, Address::globalLabel(global.name, global.sizeInBytes));
        // resolve() shell only; home is globalHomes, never register-cached.
        globals.emplace(global.name, global.toValue());
    }
}

void StackMachine::startProcedure(const Procedure& procedure) {
    const std::string& procedureName = procedure.name;
    const std::vector<Value>& values = procedure.frame.locals;
    const std::vector<Value>& arguments = procedure.frame.arguments;
    const bool variadic = procedure.variadic;
    const bool memoryReturn = procedure.memoryReturn;

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    sretSymbolName.clear();
    variadicFrame.reset();
    instructionOrdinal = 0;
    definedProcedures.insert(procedureName);
    if (procedure.exported) {
        assembly.raw(instructionSet->globl(procedureName) + "\n");
    }
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto& value : values) {
        scopeValues.insert({value.getName(), value});
    }

    // Variadic callees: GP/XMM save area so va_arg walks register-passed extras.
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();

    SysVArgCounts argCounts;
    struct IncomingRegArg {
        std::string name;
        SysVArgAssignment asgn;
    };
    std::vector<IncomingRegArg> incomingRegArgs;
    // Next free spill-slot word index. Must be max(index+words) over locals, not
    // scopeValues.size() (entry count): a multi-word local occupies several words
    // under one map entry, so using size() overlaps arg homes with the local
    // (git strbuf_init: blank is 3 words; hint's home collided and was clobbered).
    int localIndex = 0;
    for (const auto& entry : scopeValues) {
        int words = type::object_abi::valueWords(entry.second.getSizeInBytes());
        localIndex = std::max(localIndex, entry.second.getIndex() + words);
    }
    // System V memory return: first integer arg register holds the hidden pointer.
    if (memoryReturn) {
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
    if (variadic) {
        localIndex += vaGpSlots + static_cast<int>(SYSV_SSE_ARG_REGS) * vaXmmWordsEach;
    }

    std::vector<const Value*> stackArgs;
    for (auto& argument : arguments) {
        const int words = type::object_abi::valueWords(argument.getSizeInBytes());
        const SysVArgAssignment asgn = assignSysVArg(argument.getClassification(), argCounts,
                SYSV_INTEGER_ARG_REGS);
        if (asgn.onStack) {
            stackArgs.push_back(&argument);
            continue;
        }
        Value registerArgument { argument.getName(), localIndex, argument.getValueKind(),
                argument.getSizeInBytes(), argument.isSigned(), argument.getClassification() };
        scopeValues.insert({argument.getName(), registerArgument});
        localIndex += words;
        if (asgn.count == 1 && asgn.slots[0] == SysVArgSlot::IntegerReg
                && argument.getClassification().gprExtend == type::sysv::GprExtend::None) {
            integerArgRegs[asgn.indices[0]]->assign(&resolve(argument.getName()));
        } else {
            incomingRegArgs.push_back({ argument.getName(), asgn });
        }
    }

    std::vector<SysVStackArg> stackSpecs;
    stackSpecs.reserve(stackArgs.size());
    for (const Value* argument : stackArgs) {
        stackSpecs.push_back(stackArgOf(*argument));
    }
    const SysVStackLayout stackLayout = layoutSysVStackArgs(stackSpecs);
    for (std::size_t i = 0; i < stackArgs.size(); ++i) {
        const Value& argument = *stackArgs[i];
        scopeValues.insert({ argument.getName(), argument });
        registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                2 * MACHINE_WORD_SIZE + stackLayout.slots[i].offsetBytes, argument.getSizeInBytes()));
    }

    if (variadic) {
        createVaSaveHomes(vaSaveBaseIndex, vaGpHome, vaXmmHome);
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

    for (const auto& incoming : incomingRegArgs) {
        Value& home = resolve(incoming.name);
        for (int i = 0; i < incoming.asgn.count; ++i) {
            if (incoming.asgn.slots[static_cast<std::size_t>(i)] == SysVArgSlot::IntegerReg) {
                storeWord(*integerArgRegs[incoming.asgn.indices[static_cast<std::size_t>(i)]], home, i);
            } else {
                storeEightbyteFromXmm(static_cast<int>(incoming.asgn.indices[static_cast<std::size_t>(i)]),
                        home, i, {});
            }
        }
    }

    if (variadic) {
        dumpVariadicSaveArea(vaGpHome, vaXmmHome);
        variadicFrame = VariadicFrame {
                addressOf(resolve(vaGpHome[0])),
                Address::frame(FrameBase::BasePointer,
                        2 * MACHINE_WORD_SIZE + stackLayout.usedBytes, MACHINE_WORD_SIZE),
                sysvNamedGpOffset(argCounts),
                sysvNamedFpOffset(argCounts),
        };
    }
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
    frameHomes.clear();
    calleeSavedRegisters.clear();
    sretSymbolName.clear();
    variadicFrame.reset();
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
