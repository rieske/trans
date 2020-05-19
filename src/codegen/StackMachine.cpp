#include "StackMachine.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = 8;
}

namespace codegen {

StackMachine::StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions, std::unique_ptr<Amd64Registers> registers) :
        assembly { ostream },
        instructions { std::move(instructions) },
        registers { std::move(registers) }
{
}

void StackMachine::generatePreamble() {
    assembly.raw(instructions->preamble());
}

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments) {
    emptyGeneralPurposeRegisters();
    assembly.label(instructions->label(procedureName));
    assembly << instructions->push(registers->getBasePointer());
    assembly << instructions->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto& value : values) {
        scopeValues.insert(std::make_pair(value.getName(), value));
    }
    std::size_t integerArgumentRegisterIndex { 0 };
    std::size_t localIndex { scopeValues.size() };
    int argumentIndex { 0 };
    for (auto& argument : arguments) {
        if (argument.getType() == Type::INTEGRAL && integerArgumentRegisterIndex < registers->getIntegerArgumentRegisters().size()) {
            Value registerArgument { argument.getName(), static_cast<int>(localIndex), argument.getType(), argument.getSizeInBytes() };
            scopeValues.insert(std::make_pair(argument.getName(), registerArgument));
            registers->getIntegerArgumentRegisters()[integerArgumentRegisterIndex]->assign(&scopeValues.at(argument.getName()));
            ++integerArgumentRegisterIndex;
            ++localIndex;
        } else {
            Value stackArgument { argument.getName(), argumentIndex, argument.getType(), argument.getSizeInBytes() };
            scopeValues.insert(std::make_pair(argument.getName(), stackArgument));
            ++argumentIndex;
        }
    }
    if (!scopeValues.empty()) {
        localVariableStackSize = scopeValues.size() * MACHINE_WORD_SIZE;
        assembly << instructions->sub(registers->getStackPointer(), localVariableStackSize);
    }
    pushCalleeSavedRegisters();
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
    calleeSavedRegisters.clear();
}

void StackMachine::label(std::string name) {
    assembly.label(instructions->label(name));
}

void StackMachine::jump(JumpCondition jumpCondition, std::string label) {
    switch (jumpCondition) {
    case JumpCondition::IF_EQUAL:
        assembly << instructions->je(label);
        break;
    case JumpCondition::IF_NOT_EQUAL:
        assembly << instructions->jne(label);
        break;
    case JumpCondition::IF_ABOVE:
        assembly << instructions->jg(label);
        break;
    case JumpCondition::IF_BELOW:
        assembly << instructions->jl(label);
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        assembly << instructions->jge(label);
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        assembly << instructions->jle(label);
        break;
    case JumpCondition::UNCONDITIONAL:
        default:
        assembly << instructions->jmp(label);
    }
}

void StackMachine::storeGeneralPurposeRegisterValues() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::callInputProcedure(std::string symbolName) {
    storeGeneralPurposeRegisterValues();
    auto& operand = scopeValues.at(symbolName);
    assembly << instructions->mov(memoryBaseRegister(operand), *registers->getIntegerArgumentRegisters().at(1));
    int offset = memoryOffset(operand);
    if (offset) {
        assembly << instructions->add(*registers->getIntegerArgumentRegisters().at(1), offset);
    }
    assembly << instructions->mov("sfmt", *registers->getIntegerArgumentRegisters().at(0));
    assembly << instructions->xor_(registers->getRetrievalRegister(), registers->getRetrievalRegister());
    assembly << instructions->call("scanf");
}

void StackMachine::callOutputProcedure(std::string symbolName) {
    storeGeneralPurposeRegisterValues();
    assignRegisterToSymbol(*registers->getIntegerArgumentRegisters().at(1), scopeValues.at(symbolName));
    assembly << instructions->mov("fmt", *registers->getIntegerArgumentRegisters().at(0));
    assembly << instructions->xor_(registers->getRetrievalRegister(), registers->getRetrievalRegister());
    assembly << instructions->call("printf");
}

void StackMachine::assignRegisterToSymbol(Register& reg, Value& symbol) {
    if (symbol.isStored()) {
        storeRegisterValue(reg);
        assembly << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    } else if (&reg != &symbol.getAssignedRegister()) {
        storeRegisterValue(reg);
        Register& valueRegister = symbol.getAssignedRegister();
        assembly << instructions->mov(valueRegister, reg);
        valueRegister.free();
    }
}

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = scopeValues.at(leftSymbolName);
    auto& rightSymbol = scopeValues.at(rightSymbolName);

    if (leftSymbol.isStored() && rightSymbol.isStored()) {
        Register& rightSymbolRegister = assignRegisterTo(rightSymbol);
        assembly << instructions->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbolRegister);
    } else if (leftSymbol.isStored()) {
        assembly << instructions->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (rightSymbol.isStored()) {
        assembly << instructions->cmp(leftSymbol.getAssignedRegister(), memoryBaseRegister(rightSymbol), memoryOffset(rightSymbol));
    } else {
        assembly << instructions->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = scopeValues.at(symbolName);
    if (symbol.isStored()) {
        assembly << instructions->cmp(memoryBaseRegister(symbol), memoryOffset(symbol), 0);
    } else {
        assembly << instructions->cmp(symbol.getAssignedRegister(), 0);
    }
}

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    storeInMemory(operand);
    Register& resultRegister = get64BitRegister();
    assembly << instructions->mov(memoryBaseRegister(operand), resultRegister);
    int offset = memoryOffset(operand);
    if (offset) {
        assembly << instructions->add(resultRegister, offset);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assignRegisterTo(operand);
    }
    Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
    assembly << instructions->mov(operand.getAssignedRegister(), 0, resultRegister);
    resultRegister.assign(&scopeValues.at(resultName));

    Register& lvalueRegister = get64BitRegister();
    assembly << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), lvalueRegister);
    lvalueRegister.assign(&scopeValues.at(lvalueName));
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        Register& resultRegister = get64BitRegister();
        assembly << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
        assembly << instructions->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructions->mov(operandRegister, resultRegister);
        assembly << instructions->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    }
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    if (operand.isStored() && result.isStored()) {
        Register& resultRegister = assignRegisterTo(result);
        assembly << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
    } else if (operand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), result.getAssignedRegister());
    } else if (result.isStored()) {
        assembly << instructions->mov(operand.getAssignedRegister(), memoryBaseRegister(result), memoryOffset(result));
    } else {
        assembly << instructions->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto& result = scopeValues.at(resultName);
    if (result.isStored()) {
        assembly << instructions->mov(constant, memoryBaseRegister(result), memoryOffset(result)); // mov dword?
    } else {
        assembly << instructions->mov(constant, result.getAssignedRegister()); // mov dword?
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    Register& operandRegister = operand.isStored() ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = result.isStored() ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    assembly << instructions->mov(operandRegister, resultRegister, 0);
}

void StackMachine::procedureArgument(std::string argumentName) {
    auto argument = &scopeValues.at(argumentName);
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
    saveCallerSavedRegisters();
    int argumentOffset { 0 };
    for (auto argument : stackArguments) {
        pushProcedureArgument(*argument, argumentOffset + callerSavedRegisters.size() * MACHINE_WORD_SIZE);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    integerArguments.clear();
    stackArguments.clear();
    assembly << instructions->call(procedureName);
    if (argumentOffset) {
        assembly << instructions->add(registers->getStackPointer(), argumentOffset);
    }
    popCallerSavedRegisters();
}

void StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    if (symbolToPush.isStored()) {
        Register& reg = get64BitRegister();
        assembly << instructions->mov(
                memoryBaseRegister(symbolToPush),
                symbolToPush.isFunctionArgument() ? memoryOffset(symbolToPush) : memoryOffset(symbolToPush) + argumentOffset,
                reg);
        assembly << instructions->push(reg);
    } else {
        assembly << instructions->push(symbolToPush.getAssignedRegister());
    }
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value& returnSymbol = scopeValues.at(returnSymbolName);
        if (returnSymbol.isStored()) {
            assembly << instructions->mov(memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol), registers->getRetrievalRegister());
        } else if (&registers->getRetrievalRegister() != &returnSymbol.getAssignedRegister()) {
            assembly << instructions->mov(returnSymbol.getAssignedRegister(), registers->getRetrievalRegister());
        }
    }
    popCalleeSavedRegisters();
    assembly << instructions->leave();
    assembly << instructions->ret();
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value& returnSymbol = scopeValues.at(returnSymbolName);
    assembly << instructions->mov(registers->getRetrievalRegister(), memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol));
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->xor_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->xor_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructions->xor_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->xor_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->or_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->or_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructions->or_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->or_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->and_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->and_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructions->and_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->add(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->add(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructions->add(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->sub(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructions->sub(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->sub(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else {
        assembly << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructions->sub(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Value& result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error { "multiplication of non integers is not implemented" };
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    if (rightOperand.isStored()) {
        assembly << instructions->imul(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructions->imul(rightOperand.getAssignedRegister());
    }
    registers->getMultiplicationRegister().assign(&result);
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Value& result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error { "division of non integer types is not implemented" };
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructions->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
        assembly << instructions->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructions->idiv(rightOperand.getAssignedRegister());
    }
    registers->getMultiplicationRegister().assign(&result);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Value& result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error { "modular division of non integer types is not implemented" };
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructions->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructions->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
        assembly << instructions->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructions->idiv(rightOperand.getAssignedRegister());
    }
    registers->getRemainderRegister().assign(&result);
}

void StackMachine::inc(std::string operandName) {
    Value& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assembly << instructions->inc(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        assembly << instructions->inc(operand.getAssignedRegister());
    }
}

void StackMachine::dec(std::string operandName) {
    Value& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assembly << instructions->dec(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        assembly << instructions->dec(operand.getAssignedRegister());
    }
}

void StackMachine::storeRegisterValue(Register& reg) {
    if (reg.containsUnstoredValue()) {
        assembly << instructions->mov(reg, memoryBaseRegister(*reg.getValue()), memoryOffset(*reg.getValue()));
        reg.free();
    }
}

void StackMachine::emptyGeneralPurposeRegisters() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        reg->free();
    }
}

void StackMachine::saveCallerSavedRegisters() {
    storeRegisterValue(registers->getRetrievalRegister());
    pushDirtyRegisters(registers->getCallerSavedRegisters(), callerSavedRegisters);
}

void StackMachine::popCallerSavedRegisters() {
    popRegisters(callerSavedRegisters);
    callerSavedRegisters.clear();
}

void StackMachine::pushCalleeSavedRegisters() {
    pushRegisters(registers->getCalleeSavedRegisters(), calleeSavedRegisters);
}

void StackMachine::popCalleeSavedRegisters() {
    popRegisters(calleeSavedRegisters);
}

void StackMachine::pushDirtyRegisters(std::vector<Register*> source, std::vector<Register*>& destination) {
    for (auto& reg : source) {
        if (reg->containsUnstoredValue()) {
            pushRegister(*reg, destination);
        }
    }
}

void StackMachine::pushRegisters(std::vector<Register*> source, std::vector<Register*>& destination) {
    for (auto& reg : source) {
        pushRegister(*reg, destination);
    }
}

void StackMachine::popRegisters(std::vector<Register*> registers) {
    for (auto& reg : registers) {
        assembly << instructions->pop(*reg);
    }
}

void StackMachine::pushRegister(Register& reg, std::vector<Register*>& registers) {
    registers.insert(registers.begin(), &reg);
    assembly << instructions->push(reg);
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

const Register& StackMachine::memoryBaseRegister(const Value& symbol) const {
    if (symbol.isFunctionArgument()) {
        return registers->getBasePointer();
    }
    return registers->getStackPointer();
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
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (reg != &registerToExclude) {
            storeRegisterValue(*reg);
            return *reg;
        }
    }
    throw std::runtime_error { "unable to get a free register" };
}

Register& StackMachine::assignRegisterTo(Value& symbol) {
    Register& reg = get64BitRegister();
    assembly << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value& symbol, Register& registerToExclude) {
    Register& reg = get64BitRegisterExcluding(registerToExclude);
    assembly << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

} /* namespace code_generator */
