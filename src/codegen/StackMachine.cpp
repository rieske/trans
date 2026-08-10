#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <cassert>
#include <stdexcept>

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
const int STACK_ALIGNMENT = type::object_abi::STACK_ALIGNMENT;
} // namespace

namespace codegen {

StackMachine::StackMachine(std::ostream *ostream, std::unique_ptr<InstructionSet> instructionSet, std::unique_ptr<Amd64Registers> registers) :
        assembly{ostream},
        instructionSet{std::move(instructionSet)},
        registers{std::move(registers)} {}

void StackMachine::generatePreamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions) {
    assembly.raw(instructionSet->preamble(constants, globalVariables, externalFunctions));
    for (const auto& global : globalVariables) {
        globalHomes.emplace(global.name, Address::globalLabel(global.name, global.sizeInBytes));
        // resolve() shell only; home is globalHomes, never register-cached.
        globals.emplace(global.name, global.toValue());
    }
}

void StackMachine::registerDefinedProcedure(std::string procedureName) {
    definedProcedures.insert(std::move(procedureName));
}

bool StackMachine::isDefinedProcedure(const std::string& name) const {
    return definedProcedures.count(name) > 0;
}

void StackMachine::startProcedure(const Procedure& procedure) {
    const std::string& procedureName = procedure.name;
    const std::vector<Value>& values = procedure.frame.locals;
    const std::vector<Value>& arguments = procedure.frame.arguments;
    const bool memoryReturn = procedure.memoryReturn;
    const bool variadic = procedure.variadic;

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    sretSymbolName.clear();
    variadicFrame.reset();
    const std::string lastNamedFormal = arguments.empty() ? std::string {} : arguments.back().getName();
    bool lastFormalOnStack = false;
    if (procedure.exported) {
        assembly.raw(instructionSet->globl(procedureName) + "\n");
    }
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    auto wordSlots = [](const Value& v) {
        return type::object_abi::valueWords(v.getSizeInBytes());
    };
    // Highest exclusive word index among multi-word locals (index is a word offset).
    int nextLocalWord = 0;
    for (auto& value : values) {
        scopeValues.insert({value.getName(), value});
        const int end = value.getIndex() + wordSlots(value);
        if (end > nextLocalWord) {
            nextLocalWord = end;
        }
    }
    SysVArgCounts argCounts;
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t maxIntegerRegs = integerArgRegs.size();
    struct IncomingRegArg {
        std::string name;
        SysVArgAssignment asgn;
    };
    std::vector<IncomingRegArg> incomingRegArgs;
    int localIndex{nextLocalWord};
    std::vector<const Value*> stackArgs;
    if (memoryReturn) {
        sretSymbolName = type::object_abi::SRET_SYMBOL_NAME;
        Value sret { sretSymbolName, localIndex, Type::INTEGRAL, MACHINE_WORD_SIZE };
        scopeValues.insert({ sretSymbolName, sret });
        integerArgRegs[0]->assign(&resolve(sretSymbolName));
        argCounts.integerRegs = 1;
        localIndex += 1;
    }

    const int vaGpSlots = static_cast<int>(SYSV_INTEGER_ARG_REGS);
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    const int vaSaveBaseIndex = variadic ? localIndex : -1;
    std::vector<std::string> vaGpHome(SYSV_INTEGER_ARG_REGS);
    std::vector<std::string> vaXmmHome(SYSV_SSE_ARG_REGS);
    if (variadic) {
        localIndex += vaGpSlots + static_cast<int>(SYSV_SSE_ARG_REGS) * vaXmmWordsEach;
    }

    for (auto& argument : arguments) {
        const SysVArgAssignment asgn = assignSysVArg(argument.getClassification(), argCounts, maxIntegerRegs);
        lastFormalOnStack = asgn.onStack;
        if (asgn.onStack) {
            stackArgs.push_back(&argument);
            continue;
        }
        Value registerArgument { argument.getName(), localIndex, argument.getType(),
                argument.getSizeInBytes(), argument.getClassification() };
        scopeValues.insert({argument.getName(), registerArgument});
        localIndex += wordSlots(registerArgument);
        if (asgn.count == 1 && asgn.slots[0] == SysVArgSlot::IntegerReg) {
            integerArgRegs[asgn.indices[0]]->assign(&resolve(argument.getName()));
        } else {
            incomingRegArgs.push_back({ argument.getName(), asgn });
        }
    }

    std::vector<SysVStackArg> stackSpecs;
    stackSpecs.reserve(stackArgs.size());
    for (const Value* argument : stackArgs) {
        stackSpecs.push_back({ argument->getSizeInBytes(), argument->getClassification().alignBytes });
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
    // Spill region covers multi-word locals and register-passed args (not stack args).
    localVariableStackSize = localIndex * MACHINE_WORD_SIZE;
    int stackSize = savedRegistersStack + localVariableStackSize;
    if (stackSize % STACK_ALIGNMENT) {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize + MACHINE_WORD_SIZE);
    } else {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize);
    }

    pushCalleeSavedRegisters();
    // Stack-pointer locals / reg-arg spill slots include callee-saved space.
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
                        home, i);
            }
        }
    }

    if (variadic) {
        dumpVariadicSaveArea(vaGpHome, vaXmmHome);
        spillGeneralPurposeRegisters();
        emptyGeneralPurposeRegisters();
        variadicFrame = VariadicFrame {
                addressOf(resolve(vaGpHome[0])),
                Address::frame(FrameBase::BasePointer, 2 * MACHINE_WORD_SIZE, MACHINE_WORD_SIZE),
                lastNamedFormal,
                lastFormalOnStack,
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
}

void StackMachine::label(std::string name) {
    spillGeneralPurposeRegisters();
    assembly.label(instructionSet->label(name));
}

void StackMachine::jump(JumpCondition jumpCondition, std::string label) {
    // Spill on every outgoing edge. Conditional jumps used to skip this, so a branch
    // to a join label could skip the spill that label() emits only on fall-through —
    // leaving live values (e.g. argument registers) in regs while later code reloads
    // them from unsaved stack slots. Repro: `int f(int a){ if(0); return a; }`.
    spillGeneralPurposeRegisters();
    switch (jumpCondition) {
    case JumpCondition::IF_EQUAL:
        assembly << instructionSet->je(label);
        break;
    case JumpCondition::IF_NOT_EQUAL:
        assembly << instructionSet->jne(label);
        break;
    case JumpCondition::IF_ABOVE:
        assembly << instructionSet->jg(label);
        break;
    case JumpCondition::IF_BELOW:
        assembly << instructionSet->jl(label);
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        assembly << instructionSet->jge(label);
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        assembly << instructionSet->jle(label);
        break;
    case JumpCondition::UNCONDITIONAL:
    default:
        assembly << instructionSet->jmp(label);
    }
}

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
    if (isSseFloat32(symbol)) {
        assembly << instructionSet->movDword(memoryOperand(symbol), dest);
        return;
    }
    assembly << instructionSet->mov(memoryOperand(symbol), dest);
}

void StackMachine::loadWithoutBinding(Value& symbol, Register& dest) {
    emitLoad(symbol, dest);
}

void StackMachine::emitStore(Register& source, Value& symbol) {
    if (isSseFloat32(symbol)) {
        assembly << instructionSet->movDword(source, memoryOperand(symbol));
        return;
    }
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

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = resolve(leftSymbolName);
    auto& rightSymbol = resolve(rightSymbolName);

    // Usual arithmetic: promote integral side to double bits before comparing.
    // IEEE bit patterns order as signed integers for non-NaN values.
    const bool floating = leftSymbol.getType() == Type::FLOATING
            || rightSymbol.getType() == Type::FLOATING;
    if (floating) {
        const bool destFloat32 = !isSseFloat64(leftSymbol) && !isSseFloat64(rightSymbol);
        loadValueToXmm(leftSymbol, 0, destFloat32);
        loadValueToXmm(rightSymbol, 1, destFloat32);
        Register& leftReg = get64BitRegister();
        Register& rightReg = get64BitRegisterExcluding(leftReg);
        xmmToGpr(0, leftReg, destFloat32);
        xmmToGpr(1, rightReg, destFloat32);
        assembly << instructionSet->cmp(leftReg, rightReg);
        return;
    }

    if (residesInMemory(leftSymbol) && residesInMemory(rightSymbol)) {
        Register& rightSymbolRegister = assignRegisterTo(rightSymbol);
        assembly << instructionSet->cmp(memoryOperand(leftSymbol), rightSymbolRegister);
    } else if (residesInMemory(leftSymbol)) {
        assembly << instructionSet->cmp(memoryOperand(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (residesInMemory(rightSymbol)) {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), memoryOperand(rightSymbol));
    } else {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = resolve(symbolName);
    if (residesInMemory(symbol)) {
        assembly << instructionSet->cmp(memoryOperand(symbol), 0);
    } else {
        assembly << instructionSet->cmp(symbol.getAssignedRegister(), 0);
    }
}

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    storeInMemory(operand);
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(memoryOperand(operand), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::functionAddress(std::string functionName, std::string resultName) {
    Register& resultRegister = get64BitRegister();
    if (isDefinedProcedure(functionName)) {
        assembly << instructionSet->lea(MemoryOperand::global(functionName), resultRegister);
    } else {
        assembly << instructionSet->loadGot(functionName, resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);
    // Use the register returned by the load path; global pointer homes are not register-bound.
    Register& pointerRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = get64BitRegisterExcluding(pointerRegister);
    const int loadSize = result.getSizeInBytes();
    // Sign-extend into the full 64-bit register: the rest of the ALU uses 64-bit ops/cmp.
    // (Types are signed-default for char/int on this frontend.)
    if (loadSize == 1) {
        assembly << instructionSet->loadByteSignExtend(pointerRegister, resultRegister);
    } else if (loadSize == 4) {
        assembly << instructionSet->loadDwordSignExtend(pointerRegister, resultRegister);
    } else {
        assembly << instructionSet->mov(MemoryOperand::at(pointerRegister, 0), resultRegister);
    }
    bindResult(resultRegister, result);

    Register& lvalueRegister = get64BitRegisterExcluding(pointerRegister);
    assembly << instructionSet->mov(pointerRegister, lvalueRegister);
    lvalueRegister.assign(&resolve(lvalueName));
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    // IEEE float/double: flip sign bit (integer neg corrupts the bit pattern).
    if (operand.getType() == Type::FLOATING) {
        Register& resultRegister = residesInMemory(operand)
                ? get64BitRegister()
                : get64BitRegisterExcluding(operand.getAssignedRegister());
        if (residesInMemory(operand)) {
            emitLoad(operand, resultRegister);
        } else {
            assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
        }
        Register& mask = get64BitRegisterExcluding(resultRegister);
        const char* signBit = isSseFloat32(operand)
                ? "0x80000000" : "0x8000000000000000";
        assembly << instructionSet->mov(signBit, mask);
        assembly << instructionSet->xor_(mask, resultRegister);
        bindResult(resultRegister, resolve(resultName));
        return;
    }
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    }
}

void StackMachine::bswap(std::string operandName, std::string resultName, int widthBytes) {
    auto& operand = resolve(operandName);
    Register& resultRegister = residesInMemory(operand)
            ? get64BitRegister()
            : get64BitRegisterExcluding(operand.getAssignedRegister());
    if (residesInMemory(operand)) {
        emitLoad(operand, resultRegister);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), resultRegister);
    }
    for (const auto& insn : instructionSet->bswap(resultRegister, widthBytes)) {
        assembly << insn;
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::unaryNot(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    if (residesInMemory(operand)) {
        Register& resultRegister = get64BitRegister();
        emitLoad(operand, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->not_(resultRegister);
        bindResult(resultRegister, resolve(resultName));
    }
}

void StackMachine::widenInteger(std::string operandName, std::string resultName, bool isSigned) {
    Value& operand = resolve(operandName);
    Value& result = resolve(resultName);
    storeInMemory(operand);
    Register& addr = get64BitRegister();
    assembly << instructionSet->lea(memoryOperand(operand), addr);
    Register& dest = get64BitRegisterExcluding(addr);
    const int n = operand.getSizeInBytes();
    if (n <= 1) {
        if (isSigned) {
            assembly << instructionSet->loadByteSignExtend(addr, dest);
        } else {
            assembly << instructionSet->loadByteZeroExtend(addr, dest);
        }
    } else if (n <= 2) {
        if (isSigned) {
            assembly << instructionSet->loadWordSignExtend(addr, dest);
        } else {
            assembly << instructionSet->loadWordZeroExtend(addr, dest);
        }
    } else if (isSigned) {
        assembly << instructionSet->loadDwordSignExtend(addr, dest);
    } else {
        assembly << instructionSet->movDword(MemoryOperand::at(addr, 0), dest);
    }
    bindResult(dest, result);
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    if (type::object_abi::valueWords(operand.getSizeInBytes()) > 1
            || type::object_abi::valueWords(result.getSizeInBytes()) > 1) {
        copyWords(operand, result);
        return;
    }

    if (tryNumericAssignConvert(operand, result)) {
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
    // Float IEEE bits and large integers exceed signed 32-bit imm to memory; go via register.
    auto& result = resolve(resultName);
    Register& reg = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    assembly << instructionSet->mov(constant, reg);
    if (residesInMemory(result)) {
        emitStore(reg, result);
    }
}

void StackMachine::assignLabelAddress(std::string label, std::string resultName) {
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(MemoryOperand::global(label), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    Register& operandRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = residesInMemory(result) ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    // Store width follows the rvalue size so packed char/float/int elements do not clobber neighbors.
    const int storeSize = operand.getSizeInBytes();
    if (storeSize == 1) {
        assembly << instructionSet->storeByte(operandRegister, resultRegister);
    } else if (storeSize == 4) {
        assembly << instructionSet->storeDword(operandRegister, resultRegister);
    } else {
        assembly << instructionSet->mov(operandRegister, MemoryOperand::at(resultRegister, 0));
    }
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->xor_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->xor_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->or_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->or_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->and_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

namespace {

bool involvesFloating(const Value& left, const Value& right, const Value& result) {
    return left.getType() == Type::FLOATING || right.getType() == Type::FLOATING
            || result.getType() == Type::FLOATING;
}

} // namespace

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::addss, &InstructionSet::addsd);
        return;
    }

    Register& resultRegister = get64BitRegister();
    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->add(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, result);
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::subss, &InstructionSet::subsd);
        return;
    }

    Register& resultRegister = get64BitRegister();
    if (residesInMemory(leftOperand)) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->sub(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->sub(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, result);
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::mulss, &InstructionSet::mulsd);
        return;
    }

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"multiplication of non integers is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    // imul writes RDX:RAX; spill RDX if it holds a live value (e.g. pointer for *p *= ...)
    storeRegisterValue(registers->getRemainderRegister());
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->imul(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->imul(rightOperand.getAssignedRegister());
    }
    bindResult(multiplicationRegister, result);
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingBinary(leftOperand, rightOperand, result,
                &InstructionSet::divss, &InstructionSet::divsd);
        return;
    }

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    // Signed divide: sign-extend dividend in RAX into RDX:RAX (xor rdx,rdx breaks negatives).
    assembly << instructionSet->cqo();
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->idiv(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    bindResult(multiplicationRegister, result);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"modular division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructionSet->cqo();
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->idiv(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    bindResult(registers->getRemainderRegister(), result);
}

void StackMachine::inc(std::string operandName, int step) {
    Value& operand = resolve(operandName);
    if (step == 1) {
        if (residesInMemory(operand)) {
            assembly << instructionSet->inc(memoryOperand(operand));
        } else {
            assembly << instructionSet->inc(operand.getAssignedRegister());
        }
        return;
    }
    // Non-unit step: pointer ++ (byte stride = sizeof *p), not scalar +1.
    Register& reg = get64BitRegister();
    assignRegisterToSymbol(reg, operand);
    assembly << instructionSet->add(reg, step);
    emitStore(reg, operand);
    bindResult(reg, operand);
}

void StackMachine::dec(std::string operandName, int step) {
    Value& operand = resolve(operandName);
    if (step == 1) {
        if (residesInMemory(operand)) {
            assembly << instructionSet->dec(memoryOperand(operand));
        } else {
            assembly << instructionSet->dec(operand.getAssignedRegister());
        }
        return;
    }
    // Non-unit step: pointer -- (byte stride = sizeof *p).
    Register& reg = get64BitRegister();
    assignRegisterToSymbol(reg, operand);
    assembly << instructionSet->sub(reg, step);
    emitStore(reg, operand);
    bindResult(reg, operand);
}

void StackMachine::shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
        std::string (InstructionSet::*emitShift)(const Register&) const) {
    // Count must live in %cl (RCX) and be tracked so the value is not placed in RCX.
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = resolve(rightOperandName);
    if (residesInMemory(rightOperand)) {
        emitLoad(rightOperand, counterRegister);
    } else if (&counterRegister != &rightOperand.getAssignedRegister()) {
        assembly << instructionSet->mov(rightOperand.getAssignedRegister(), counterRegister);
        storeRegisterValue(rightOperand.getAssignedRegister());
    }
    // Count in %cl is scratch only; never register-cache a global home on RCX.
    if (!addressOf(rightOperand).isGlobal()) {
        counterRegister.assign(&rightOperand);
    }

    Value& leftOperand = resolve(leftOperandName);
    Register& resultRegister = get64BitRegisterExcluding(counterRegister);
    assignRegisterToSymbol(resultRegister, leftOperand);
    assembly << (instructionSet.get()->*emitShift)(resultRegister);
    Value& result = resolve(resultName);
    bindResult(resultRegister, result);
}

void StackMachine::shl(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shl);
}

void StackMachine::shr(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shr);
}

void StackMachine::storeRegisterValue(Register& reg) {
    if (reg.containsUnstoredValue()) {
        emitStore(reg, *reg.getValue());
        reg.free();
    }
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

void StackMachine::storeInMemory(Value& symbol) {
    if (!symbol.isStored()) {
        storeRegisterValue(symbol.getAssignedRegister());
    }
}

Address StackMachine::spillSlotAddress(const Value& symbol) const {
    int offset = symbol.getIndex() * MACHINE_WORD_SIZE
            + static_cast<int>(calleeSavedRegisters.size()) * MACHINE_WORD_SIZE;
    return Address::frame(FrameBase::StackPointer, offset, symbol.getSizeInBytes());
}

void StackMachine::registerFrameHome(const std::string& name, Address address) {
    [[maybe_unused]] const bool inserted =
            frameHomes.emplace(name, std::move(address)).second;
    assert(inserted && "duplicate frame home registration");
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
    return get64BitRegisterExcluding(std::vector<Register*> { &registerToExclude });
}

Register& StackMachine::get64BitRegisterExcluding(const std::vector<Register*>& exclude) {
    auto excluded = [&](Register* reg) {
        for (Register* skip : exclude) {
            if (reg == skip) {
                return true;
            }
        }
        return false;
    };
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!excluded(reg) && !reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!excluded(reg)) {
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

void StackMachine::setScope(std::vector<Value> variables) {
    for (auto& var : variables) {
        scopeValues.insert({var.getName(), var});
        registerFrameHome(var.getName(), spillSlotAddress(var));
    }
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
    throw std::runtime_error { "codegen: no storage for symbol `" + name + "` (function designator or missing global?)" };
}

} // namespace codegen

