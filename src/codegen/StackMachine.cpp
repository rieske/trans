#include "StackMachine.h"

#include <cassert>
#include <cctype>
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
    for (const auto& global : globalVariables) {
        globalHomes.emplace(global.name, Address::globalLabel(global.name, global.sizeInBytes));
        // resolve() shell only; home is globalHomes, never register-cached.
        globals.emplace(global.name, global.toValue());
    }
}

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments) {

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    auto wordSlots = [](const Value& v) {
        const int size = v.getSizeInBytes();
        if (size <= 0) {
            return 1;
        }
        return (size + MACHINE_WORD_SIZE - 1) / MACHINE_WORD_SIZE;
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
    std::size_t integerArgumentRegisterIndex{0};
    int localIndex{nextLocalWord};
    int argumentIndex{0};
    for (auto& argument : arguments) {
        if (argument.getType() == Type::INTEGRAL && integerArgumentRegisterIndex < registers->getIntegerArgumentRegisters().size()) {
            Value registerArgument{argument.getName(), localIndex, argument.getType(), argument.getSizeInBytes()};
            scopeValues.insert({argument.getName(), registerArgument});
            registers->getIntegerArgumentRegisters()[integerArgumentRegisterIndex]->assign(&resolve(argument.getName()));
            ++integerArgumentRegisterIndex;
            localIndex += wordSlots(registerArgument);
        } else {
            Value stackArgument{argument.getName(), argumentIndex, argument.getType(), argument.getSizeInBytes()};
            scopeValues.insert({argument.getName(), stackArgument});
            // Stack args live at fixed base-pointer offsets; independent of callee-saved size.
            registerFrameHome(argument.getName(), Address::frame(FrameBase::BasePointer,
                    (argumentIndex + 2) * MACHINE_WORD_SIZE, argument.getSizeInBytes()));
            ++argumentIndex;
        }
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
    assembly << instructionSet->mov(memoryOperand(symbol), dest);
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

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = resolve(leftSymbolName);
    auto& rightSymbol = resolve(rightSymbolName);

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

void StackMachine::indexAddress(std::string baseName, std::string indexName, int elementSizeBytes, std::string resultName,
        bool baseIsArray) {
    auto& base = resolve(baseName);
    auto& index = resolve(indexName);

    // One-operand imul writes RDX:RAX. Scale index first, then form base in another reg.
    Register& mulReg = registers->getMultiplicationRegister();
    Register& rdx = registers->getRemainderRegister();
    storeRegisterValue(mulReg);
    storeRegisterValue(rdx);

    if (elementSizeBytes != 1) {
        assignRegisterToSymbol(mulReg, index);
        Register& sizeReg = get64BitRegisterExcluding(mulReg);
        if (&sizeReg == &rdx) {
            // Prefer a third register if get64BitRegisterExcluding only avoids mulReg.
            storeRegisterValue(rdx);
        }
        Register& scaleReg = get64BitRegisterExcluding(mulReg);
        assembly << instructionSet->mov(std::to_string(elementSizeBytes), scaleReg);
        assembly << instructionSet->imul(scaleReg);
    }

    Register& addr = get64BitRegisterExcluding(mulReg);
    if (baseIsArray) {
        storeInMemory(base);
        assembly << instructionSet->lea(memoryOperand(base), addr);
    } else if (residesInMemory(base)) {
        emitLoad(base, addr);
    } else {
        assembly << instructionSet->mov(base.getAssignedRegister(), addr);
    }

    if (elementSizeBytes == 1) {
        if (residesInMemory(index)) {
            assembly << instructionSet->add(memoryOperand(index), addr);
        } else {
            assembly << instructionSet->add(index.getAssignedRegister(), addr);
        }
    } else {
        assembly << instructionSet->add(mulReg, addr);
    }
    bindResult(addr, resolve(resultName));
}

namespace {
// Low-byte name for NASM size-qualified stores (rax→al, r8→r8b, …).
std::string lowByteName(const Register& reg) {
    const std::string n = reg.getName();
    if (n == "rax") return "al";
    if (n == "rbx") return "bl";
    if (n == "rcx") return "cl";
    if (n == "rdx") return "dl";
    if (n == "rsi") return "sil";
    if (n == "rdi") return "dil";
    if (n == "rbp") return "bpl";
    if (n == "rsp") return "spl";
    if (n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]))) {
        return n + "b";
    }
    return n;
}
std::string lowDwordName(const Register& reg) {
    const std::string n = reg.getName();
    if (n == "rax") return "eax";
    if (n == "rbx") return "ebx";
    if (n == "rcx") return "ecx";
    if (n == "rdx") return "edx";
    if (n == "rsi") return "esi";
    if (n == "rdi") return "edi";
    if (n == "rbp") return "ebp";
    if (n == "rsp") return "esp";
    // r8–r15: dword form is r8d…r15d (not e8).
    if (n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]))) {
        return n + "d";
    }
    return n;
}
} // namespace

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);
    // Use the register returned by the load path; global pointer homes are not register-bound.
    Register& pointerRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = get64BitRegisterExcluding(pointerRegister);
    const int loadSize = result.getSizeInBytes();
    if (loadSize == 1) {
        // Zero-extend byte load so high bits don't pollute int uses (printf %d).
        assembly << ("movzx " + resultRegister.getName() + ", byte [" + pointerRegister.getName() + "]");
    } else if (loadSize == 4) {
        assembly << ("mov " + lowDwordName(resultRegister) + ", dword [" + pointerRegister.getName() + "]");
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

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

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
    if (residesInMemory(result)) {
        assembly << instructionSet->mov(constant, memoryOperand(result));
    } else {
        assembly << instructionSet->mov(constant, result.getAssignedRegister());
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    Register& operandRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = residesInMemory(result) ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    // Store width follows the rvalue size so packed char/int array elements do not clobber neighbors.
    const int storeSize = operand.getSizeInBytes();
    if (storeSize == 1) {
        assembly << ("mov byte [" + resultRegister.getName() + "], " + lowByteName(operandRegister));
    } else if (storeSize == 4) {
        assembly << ("mov dword [" + resultRegister.getName() + "], " + lowDwordName(operandRegister));
    } else {
        assembly << instructionSet->mov(operandRegister, MemoryOperand::at(resultRegister, 0));
    }
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
    if (residesInMemory(symbolToPush)) {
        Register& reg = get64BitRegister();
        // Stack-pointer homes move as call args are pushed; base-pointer / global do not.
        Address home = addressOf(symbolToPush);
        if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer) {
            home = Address::frame(home.frameBase(), home.offsetBytes() + argumentOffset, home.sizeBytes());
        }
        assembly << instructionSet->mov(memoryOperand(home), reg);
        assembly << instructionSet->push(reg);
    } else {
        assembly << instructionSet->push(symbolToPush.getAssignedRegister());
    }
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = resolve(returnSymbolName);
        if (residesInMemory(returnSymbol)) {
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

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
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
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
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

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    Register& multiplicationRegister = registers->getMultiplicationRegister();
    assignRegisterToSymbol(multiplicationRegister, leftOperand);
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
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
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (residesInMemory(rightOperand)) {
        assembly << instructionSet->idiv(memoryOperand(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    bindResult(registers->getRemainderRegister(), result);
}

void StackMachine::inc(std::string operandName) {
    Value& operand = resolve(operandName);
    if (residesInMemory(operand)) {
        assembly << instructionSet->inc(memoryOperand(operand));
    } else {
        assembly << instructionSet->inc(operand.getAssignedRegister());
    }
}

void StackMachine::dec(std::string operandName) {
    Value& operand = resolve(operandName);
    if (residesInMemory(operand)) {
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

