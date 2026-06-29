#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = 8;
const int STACK_ALIGNMENT = 2 * MACHINE_WORD_SIZE;
} // namespace

namespace codegen {

StackMachine::StackMachine(std::ostream *ostream, std::unique_ptr<InstructionSet> instructionSet, std::unique_ptr<Amd64Registers> registers) :
        assembly{ostream},
        instructionSet{std::move(instructionSet)},
        registers{std::move(registers)} {}

void StackMachine::generatePreamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables) {
    assembly.raw(instructionSet->preamble(constants, globalVariables));
    // Global object homes + Value caches for resolve() (register assign still needs a Value).
    for (const auto& global : globalVariables) {
        globalHomes.emplace(global.name, Address::globalLabel(global.name, global.sizeInBytes));
        globals.emplace(global.name, global.toValue());
    }
}

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments) {

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto& value : values) {
        scopeValues.insert({value.getName(), value});
    }
    std::size_t integerArgumentRegisterIndex{0};
    std::size_t localIndex{scopeValues.size()};
    int argumentIndex{0};
    for (auto& argument : arguments) {
        if (argument.getType() == Type::INTEGRAL && integerArgumentRegisterIndex < registers->getIntegerArgumentRegisters().size()) {
            Value registerArgument{argument.getName(), static_cast<int>(localIndex), argument.getType(), argument.getSizeInBytes()};
            scopeValues.insert({argument.getName(), registerArgument});
            registers->getIntegerArgumentRegisters()[integerArgumentRegisterIndex]->assign(&resolve(argument.getName()));
            ++integerArgumentRegisterIndex;
            ++localIndex;
        } else {
            Value stackArgument{argument.getName(), argumentIndex, argument.getType(), argument.getSizeInBytes(), true};
            scopeValues.insert({argument.getName(), stackArgument});
            ++argumentIndex;
        }
    }
    int savedRegistersStack = registers->getCalleeSavedRegisters().size() * MACHINE_WORD_SIZE;
    localVariableStackSize = (scopeValues.size() - argumentIndex) * MACHINE_WORD_SIZE;
    int stackSize = savedRegistersStack + localVariableStackSize;
    if (stackSize % STACK_ALIGNMENT) {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize + MACHINE_WORD_SIZE);
    } else {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize);
    }

    pushCalleeSavedRegisters();
    // Frame offsets include callee-saved space; register homes only after that is known.
    for (const auto& entry : scopeValues) {
        registerFrameHome(entry.second);
    }
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
    frameHomes.clear();
    calleeSavedRegisters.clear();
}

void StackMachine::label(std::string name) {
    spillGeneralPurposeRegisters();
    assembly.label(instructionSet->label(name));
}

void StackMachine::jump(JumpCondition jumpCondition, std::string label) {
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
        spillGeneralPurposeRegisters();
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
    assembly << instructionSet->mov(memoryOperand(symbol), dest);
}

void StackMachine::emitStore(Register& source, Value& symbol) {
    assembly << instructionSet->mov(source, memoryOperand(symbol));
}

// Bind a freshly computed result to its destination symbol. Global object homes are never
// register-resident (commit via Address); locals/temps stay register-resident for lazy write-back.
void StackMachine::bindResult(Register& reg, Value& result) {
    if (isGlobalHome(result.getName())) {
        emitStore(reg, result);
    } else {
        reg.assign(&result);
    }
}

void StackMachine::assignRegisterToSymbol(Register& reg, Value& symbol) {
    if (symbol.isStored()) {
        storeRegisterValue(reg);
        emitLoad(symbol, reg);
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

    if (leftSymbol.isStored() && rightSymbol.isStored()) {
        Register& rightSymbolRegister = assignRegisterTo(rightSymbol);
        assembly << instructionSet->cmp(memoryOperand(leftSymbol), rightSymbolRegister);
    } else if (leftSymbol.isStored()) {
        assembly << instructionSet->cmp(memoryOperand(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (rightSymbol.isStored()) {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), memoryOperand(rightSymbol));
    } else {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = resolve(symbolName);
    if (symbol.isStored()) {
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

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto& operand = resolve(operandName);
    if (operand.isStored()) {
        assignRegisterTo(operand);
    }
    Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
    assembly << instructionSet->mov(MemoryOperand::at(operand.getAssignedRegister(), 0), resultRegister);
    bindResult(resultRegister, resolve(resultName));

    Register& lvalueRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
    assembly << instructionSet->mov(operand.getAssignedRegister(), lvalueRegister);
    lvalueRegister.assign(&resolve(lvalueName));
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    if (operand.isStored()) {
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

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    if (operand.isStored() && result.isStored()) {
        Register& reg = get64BitRegister();
        emitLoad(operand, reg);
        emitStore(reg, result);
    } else if (operand.isStored()) {
        emitLoad(operand, result.getAssignedRegister());
    } else if (result.isStored()) {
        emitStore(operand.getAssignedRegister(), result);
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto& result = resolve(resultName);
    if (result.isStored()) {
        assembly << instructionSet->mov(constant, memoryOperand(result));
    } else {
        assembly << instructionSet->mov(constant, result.getAssignedRegister());
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    Register& operandRegister = operand.isStored() ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = result.isStored() ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    assembly << instructionSet->mov(operandRegister, MemoryOperand::at(resultRegister, 0));
}

void StackMachine::procedureArgument(std::string argumentName) {
    auto argument = &resolve(argumentName);
    if (integerArguments.size() < registers->getIntegerArgumentRegisters().size()) {
        integerArguments.push_back(argument);
    } else {
        stackArguments.insert(stackArguments.begin(), argument);
    }
}

void StackMachine::callProcedure(std::string procedureName) {
    for (std::size_t i = 0; i < integerArguments.size(); ++i) {
        assignRegisterToSymbol(*registers->getIntegerArgumentRegisters()[i], *integerArguments[i]);
    }
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();
    int argumentOffset{0};
    // System V AMD64: RSP must be 16-byte aligned before call. Without stack args we are
    // aligned; each stack arg is 8 bytes, so an odd count needs 8 bytes of padding.
    if (stackArguments.size() % 2 == 1) {
        assembly << instructionSet->sub(registers->getStackPointer(), MACHINE_WORD_SIZE);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    for (auto argument : stackArguments) {
        pushProcedureArgument(*argument, argumentOffset);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    integerArguments.clear();
    stackArguments.clear();
    // AL must hold the number of vector registers used for variadic calls (System V AMD64).
    // This compiler only passes integer args, so set AL to 0 via xor rax, rax.
    auto& retrievalRegister = registers->getRetrievalRegister();
    assembly << instructionSet->xor_(retrievalRegister, retrievalRegister);
    assembly << instructionSet->call(procedureName);
    if (argumentOffset) {
        assembly << instructionSet->add(registers->getStackPointer(), argumentOffset);
    }
}

void StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    if (symbolToPush.isStored()) {
        Register& reg = get64BitRegister();
        // rsp moves as stack args are pushed, so a non-argument stack operand needs the running offset.
        int extraOffset = symbolToPush.isFunctionArgument() ? 0 : argumentOffset;
        assembly << instructionSet->mov(memoryOperand(symbolToPush, extraOffset), reg);
        assembly << instructionSet->push(reg);
    } else {
        assembly << instructionSet->push(symbolToPush.getAssignedRegister());
    }
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        if (returnSymbol.isStored()) {
            emitLoad(returnSymbol, registers->getRetrievalRegister());
        } else if (&registers->getRetrievalRegister() != &returnSymbol.getAssignedRegister()) {
            assembly << instructionSet->mov(returnSymbol.getAssignedRegister(), registers->getRetrievalRegister());
        }
    }
    popCalleeSavedRegisters();
    assembly << instructionSet->leave();
    assembly << instructionSet->ret();
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value& returnSymbol = resolve(returnSymbolName);
    emitStore(registers->getRetrievalRegister(), returnSymbol);
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored()) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (rightOperand.isStored()) {
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

    if (leftOperand.isStored()) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (rightOperand.isStored()) {
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

    if (leftOperand.isStored()) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (rightOperand.isStored()) {
        assembly << instructionSet->and_(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored()) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (rightOperand.isStored()) {
        assembly << instructionSet->add(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored()) {
        emitLoad(leftOperand, resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    if (rightOperand.isStored()) {
        assembly << instructionSet->sub(memoryOperand(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->sub(rightOperand.getAssignedRegister(), resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"multiplication of non integers is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    // imul writes RDX:RAX; spill RDX if it holds a live value (e.g. pointer for *p *= ...)
    storeRegisterValue(registers->getRemainderRegister());
    if (rightOperand.isStored()) {
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

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
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
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
        assembly << instructionSet->idiv(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    bindResult(registers->getRemainderRegister(), result);
}

void StackMachine::inc(std::string operandName) {
    Value& operand = resolve(operandName);
    if (operand.isStored()) {
        assembly << instructionSet->inc(memoryOperand(operand));
    } else {
        assembly << instructionSet->inc(operand.getAssignedRegister());
    }
}

void StackMachine::dec(std::string operandName) {
    Value& operand = resolve(operandName);
    if (operand.isStored()) {
        assembly << instructionSet->dec(memoryOperand(operand));
    } else {
        assembly << instructionSet->dec(operand.getAssignedRegister());
    }
}

void StackMachine::shiftBy(std::string leftOperandName, std::string rightOperandName, std::string resultName,
        std::string (InstructionSet::*emitShift)(const Register&) const) {
    // Count must live in %cl (RCX) and be tracked so the value is not placed in RCX.
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = resolve(rightOperandName);
    if (rightOperand.isStored()) {
        emitLoad(rightOperand, counterRegister);
    } else if (&counterRegister != &rightOperand.getAssignedRegister()) {
        assembly << instructionSet->mov(rightOperand.getAssignedRegister(), counterRegister);
        storeRegisterValue(rightOperand.getAssignedRegister());
    }
    counterRegister.assign(&rightOperand);

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

int StackMachine::memoryOffset(const Value& symbol) const {
    if (symbol.isFunctionArgument()) {
        return (symbol.getIndex() + 2) * MACHINE_WORD_SIZE;
    }
    return symbol.getIndex() * MACHINE_WORD_SIZE + calleeSavedRegisters.size() * MACHINE_WORD_SIZE;
}

Address StackMachine::addressFromValueFlags(const Value& symbol, int extraOffset) const {
    if (symbol.isGlobal()) {
        return Address::globalLabel(symbol.getName(), symbol.getSizeInBytes());
    }
    const FrameBase base = symbol.isFunctionArgument() ? FrameBase::Rbp : FrameBase::Rsp;
    return Address::frame(base, memoryOffset(symbol) + extraOffset, symbol.getSizeInBytes());
}

void StackMachine::registerFrameHome(const Value& symbol) {
    frameHomes.insert_or_assign(symbol.getName(), addressFromValueFlags(symbol, 0));
}

bool StackMachine::hasObjectHome(const std::string& name) const {
    return frameHomes.count(name) || globalHomes.count(name);
}

bool StackMachine::isGlobalHome(const std::string& name) const {
    return globalHomes.count(name) > 0;
}

Address StackMachine::addressOf(const Value& symbol, int extraOffset) const {
    const std::string& name = symbol.getName();
    auto frame = frameHomes.find(name);
    if (frame != frameHomes.end()) {
        const Address& home = frame->second;
        if (extraOffset == 0) {
            return home;
        }
        return Address::frame(home.frameBase(), home.offsetBytes() + extraOffset, home.sizeBytes());
    }
    auto global = globalHomes.find(name);
    if (global != globalHomes.end()) {
        return global->second;
    }
    // Pure temps (and any name without a registered home): derive from Value flags if set.
    return addressFromValueFlags(symbol, extraOffset);
}

MemoryOperand StackMachine::memoryOperand(const Address& address) const {
    if (address.isGlobal()) {
        return MemoryOperand::global(address.label());
    }
    const Register& base = address.frameBase() == FrameBase::Rbp ?
            registers->getBasePointer() : registers->getStackPointer();
    return MemoryOperand::at(base, address.offsetBytes());
}

MemoryOperand StackMachine::memoryOperand(const Value& symbol, int extraOffset) const {
    return memoryOperand(addressOf(symbol, extraOffset));
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

Register& StackMachine::getCounterRegister() {
    Register& counter = registers->getCounterRegister();
    storeRegisterValue(counter);
    return counter;
}

Register& StackMachine::assignRegisterTo(Value& symbol) {
    Register& reg = get64BitRegister();
    emitLoad(symbol, reg);
    reg.assign(&symbol);
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value& symbol, Register& registerToExclude) {
    Register& reg = get64BitRegisterExcluding(registerToExclude);
    emitLoad(symbol, reg);
    reg.assign(&symbol);
    return reg;
}

void StackMachine::setScope(std::vector<Value> variables) {
    for (auto& var : variables) {
        scopeValues.insert({var.getName(), var});
        registerFrameHome(var);
    }
}

Value& StackMachine::resolve(const std::string& name) {
    auto local = scopeValues.find(name);
    if (local != scopeValues.end()) {
        return local->second;
    }
    return globals.at(name);
}

} // namespace codegen

