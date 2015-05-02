#include "StackMachine.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "InstructionSet.h"

namespace {
const int MACHINE_WORD_SIZE = 8;
}

namespace codegen {

StackMachine::StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions) :
        ostream { ostream },
        instructions { std::move(instructions) }
{
    *ostream << this->instructions->preamble();
}

void StackMachine::startProcedure(std::string procedureName) {
    emptyGeneralPurposeRegisters();
    *ostream << instructions->label(procedureName);
    *ostream << "\t" << instructions->push(basePointer);
    *ostream << "\t" << instructions->mov(stackPointer, basePointer);
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
}

void StackMachine::label(std::string name) const {
    *ostream << instructions->label(name);
}

void StackMachine::jump(JumpCondition jumpCondition, std::string label) {
    switch (jumpCondition) {
    case JumpCondition::IF_EQUAL:
        *ostream << "\t" << instructions->je(label);
        break;
    case JumpCondition::IF_NOT_EQUAL:
        *ostream << "\t" << instructions->jne(label);
        break;
    case JumpCondition::IF_ABOVE:
        *ostream << "\t" << instructions->jg(label);
        break;
    case JumpCondition::IF_BELOW:
        *ostream << "\t" << instructions->jl(label);
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        *ostream << "\t" << instructions->jge(label);
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        *ostream << "\t" << instructions->jle(label);
        break;
    case JumpCondition::UNCONDITIONAL:
        default:
        *ostream << "\t" << instructions->jmp(label);
    }
}

void StackMachine::allocateStack(std::vector<Value> values, std::vector<Value> arguments) {
    storeGeneralPurposeRegisterValues();
    *ostream << "\t" << instructions->sub(stackPointer, values.size() * MACHINE_WORD_SIZE);
    for (auto& value : values) {
        scopeValues.insert(std::make_pair(value.getName(), value));
    }
    for (auto& argument : arguments) {
        scopeValues.insert(std::make_pair(argument.getName(), argument));
    }
}

void StackMachine::deallocateStack() {
    *ostream << "\t" << instructions->add(stackPointer, scopeValues.size() * MACHINE_WORD_SIZE);
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
}

void StackMachine::storeGeneralPurposeRegisterValues() {
    for (auto& reg : generalPurposeRegisters) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::callInputProcedure(std::string symbolName) {
    storeGeneralPurposeRegisterValues();
    auto& operand = scopeValues.at(symbolName);
    *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), rsi);
    int offset = memoryOffset(operand);
    if (offset) {
        *ostream << "\t" << instructions->add(rsi, offset);
    }
    *ostream << "\t" << instructions->mov("sfmt", rdi);
    *ostream << "\t" << instructions->xor_(rax, rax);
    *ostream << "\t" << instructions->call("scanf");
}

void StackMachine::callOutputProcedure(std::string symbolName) {
    storeGeneralPurposeRegisterValues();
    assignRegisterToSymbol(rsi, scopeValues.at(symbolName));
    *ostream << "\t" << instructions->mov("fmt", rdi);
    *ostream << "\t" << instructions->xor_(rax, rax);
    *ostream << "\t" << instructions->call("printf");
}

void StackMachine::assignRegisterToSymbol(Register& reg, Value& symbol) {
    if (symbol.isStored()) {
        storeRegisterValue(reg);
        *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    } else if (&reg != &symbol.getAssignedRegister()) {
        storeRegisterValue(reg);
        Register& valueRegister = symbol.getAssignedRegister();
        *ostream << "\t" << instructions->mov(valueRegister, reg);
        valueRegister.free();
    }
}

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = scopeValues.at(leftSymbolName);
    auto& rightSymbol = scopeValues.at(rightSymbolName);

    if (leftSymbol.isStored() && rightSymbol.isStored()) {
        Register& rightSymbolRegister = assignRegisterTo(rightSymbol);
        *ostream << "\t" << instructions->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbolRegister);
    } else if (leftSymbol.isStored()) {
        *ostream << "\t" << instructions->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (rightSymbol.isStored()) {
        *ostream << "\t" << instructions->cmp(leftSymbol.getAssignedRegister(), memoryBaseRegister(rightSymbol), memoryOffset(rightSymbol));
    } else {
        *ostream << "\t" << instructions->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = scopeValues.at(symbolName);
    if (symbol.isStored()) {
        *ostream << "\t" << instructions->cmp(memoryBaseRegister(symbol), memoryOffset(symbol), 0);
    } else {
        *ostream << "\t" << instructions->cmp(symbol.getAssignedRegister(), 0);
    }
}

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    storeInMemory(operand);
    Register& resultRegister = get64BitRegister();
    *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), resultRegister);
    int offset = memoryOffset(operand);
    if (offset) {
        *ostream << "\t" << instructions->add(resultRegister, offset);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assignRegisterTo(operand);
    }
    Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
    *ostream << "\t" << instructions->mov(operand.getAssignedRegister(), 0, resultRegister);
    resultRegister.assign(&scopeValues.at(resultName));

    Register& lvalueRegister = get64BitRegister();
    *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), lvalueRegister);
    lvalueRegister.assign(&scopeValues.at(lvalueName));
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        Register& resultRegister = get64BitRegister();
        *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
        *ostream << "\t" << instructions->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    } else {
        Register& operandRegister = operand.getAssignedRegister();
        Register& resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        *ostream << "\t" << instructions->mov(operandRegister, resultRegister);
        *ostream << "\t" << instructions->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    }
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    if (operand.isStored() && result.isStored()) {
        Register& resultRegister = assignRegisterTo(result);
        *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
    } else if (operand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), result.getAssignedRegister());
    } else if (result.isStored()) {
        *ostream << "\t" << instructions->mov(operand.getAssignedRegister(), memoryBaseRegister(result), memoryOffset(result));
    } else {
        *ostream << "\t" << instructions->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto& result = scopeValues.at(resultName);
    if (result.isStored()) {
        *ostream << "\t" << instructions->mov(constant, memoryBaseRegister(result), memoryOffset(result)); // mov dword?
    } else {
        *ostream << "\t" << instructions->mov(constant, result.getAssignedRegister()); // mov dword?
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    Register& operandRegister = operand.isStored() ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = result.isStored() ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    *ostream << "\t" << instructions->mov(operandRegister, resultRegister, 0);
}

void StackMachine::procedureArgument(std::string argumentName) {
    argumentNames.insert(argumentNames.begin(), argumentName);
}

void StackMachine::callProcedure(std::string procedureName) {
    storeGeneralPurposeRegisterValues();
    int argumentOffset = 0;
    for (auto argumentName : argumentNames) {
        pushProcedureArgument(scopeValues.at(argumentName), argumentOffset);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    argumentNames.clear();
    *ostream << "\t" << instructions->call(procedureName);
    *ostream << "\t" << instructions->add(stackPointer, argumentOffset); // ? add esp, byte " << argumentOffset
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    Value& returnSymbol = scopeValues.at(returnSymbolName);
    if (returnSymbol.isStored()) {
        *ostream << "\t" << instructions->mov( // ?mov eax, dword " << getMemoryAddress(arg)
                memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol), *retrievalRegister);
    } else if (retrievalRegister != &returnSymbol.getAssignedRegister()) {
        *ostream << "\t" << instructions->mov(returnSymbol.getAssignedRegister(), *retrievalRegister);
    }
    *ostream << "\t" << instructions->leave();
    *ostream << "\t" << instructions->ret() << "\n";
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value& returnSymbol = scopeValues.at(returnSymbolName);
    *ostream << "\t" << instructions->mov(*retrievalRegister, memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol));
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->xor_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->xor_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        *ostream << "\t" << instructions->xor_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        *ostream << "\t" << instructions->xor_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->or_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->or_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        *ostream << "\t" << instructions->or_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        *ostream << "\t" << instructions->or_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->and_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->and_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        *ostream << "\t" << instructions->and_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        *ostream << "\t" << instructions->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->add(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->add(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        *ostream << "\t" << instructions->add(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        *ostream << "\t" << instructions->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Register& resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->sub(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        *ostream << "\t" << instructions->sub(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        *ostream << "\t" << instructions->sub(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), resultRegister);
        *ostream << "\t" << instructions->sub(rightOperand.getAssignedRegister(), resultRegister);
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
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), *multiplicationRegister);
    } else if (&leftOperand.getAssignedRegister() != multiplicationRegister) {
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), *multiplicationRegister);
    }
    if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->imul(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        *ostream << "\t" << instructions->imul(rightOperand.getAssignedRegister());
    }
    multiplicationRegister->assign(&result);
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Value& result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error { "division of non integer types is not implemented" };
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), *multiplicationRegister);
    } else if (&leftOperand.getAssignedRegister() != multiplicationRegister) {
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), *multiplicationRegister);
    }
    storeRegisterValue(*remainderRegister);
    *ostream << "\t" << instructions->xor_(*remainderRegister, *remainderRegister);
    if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        *ostream << "\t" << instructions->idiv(rightOperand.getAssignedRegister());
    }
    multiplicationRegister->assign(&result);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value& leftOperand = scopeValues.at(leftOperandName);
    Value& rightOperand = scopeValues.at(rightOperandName);
    Value& result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error { "modular division of non integer types is not implemented" };
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), *multiplicationRegister);
    } else if (&leftOperand.getAssignedRegister() != multiplicationRegister) {
        storeRegisterValue(*multiplicationRegister);
        *ostream << "\t" << instructions->mov(leftOperand.getAssignedRegister(), *multiplicationRegister);
    }
    storeRegisterValue(*remainderRegister);
    *ostream << "\t" << instructions->xor_(*remainderRegister, *remainderRegister);
    if (rightOperand.isStored()) {
        *ostream << "\t" << instructions->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        *ostream << "\t" << instructions->idiv(rightOperand.getAssignedRegister());
    }
    remainderRegister->assign(&result);
}

void StackMachine::inc(std::string operandName) {
    Value& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        *ostream << "\t" << instructions->inc(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        *ostream << "\t" << instructions->inc(operand.getAssignedRegister());
    }
}

void StackMachine::dec(std::string operandName) {
    Value& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        *ostream << "\t" << instructions->dec(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        *ostream << "\t" << instructions->dec(operand.getAssignedRegister());
    }
}

void StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    if (symbolToPush.isStored()) {
        Register& reg = get64BitRegister();
        *ostream << "\t" << instructions->mov(
                memoryBaseRegister(symbolToPush),
                symbolToPush.isFunctionArgument() ? memoryOffset(symbolToPush) : memoryOffset(symbolToPush) + argumentOffset,
                reg);
        *ostream << "\t" << instructions->push(reg);
    } else {
        *ostream << "\t" << instructions->push(symbolToPush.getAssignedRegister());
    }
}

void StackMachine::storeRegisterValue(Register& reg) {
    if (reg.containsUnstoredValue()) {
        *ostream << "\t" << instructions->mov(reg, memoryBaseRegister(*reg.getValue()), memoryOffset(*reg.getValue()));
        reg.free();
    }
}

void StackMachine::emptyGeneralPurposeRegisters() {
    for (auto& reg : generalPurposeRegisters) {
        reg->free();
    }
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
    return symbol.getIndex() * MACHINE_WORD_SIZE;
}

const Register& StackMachine::memoryBaseRegister(const Value& symbol) const {
    if (symbol.isFunctionArgument()) {
        return basePointer;
    }
    return stackPointer;
}

Register& StackMachine::get64BitRegister() {
    for (auto& reg : generalPurposeRegisters) {
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    Register& reg = **generalPurposeRegisters.begin();
    storeRegisterValue(reg);
    return reg;
}

Register& StackMachine::get64BitRegisterExcluding(Register& registerToExclude) {
    for (auto& reg : generalPurposeRegisters) {
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto& reg : generalPurposeRegisters) {
        if (reg != &registerToExclude) {
            storeRegisterValue(*reg);
            return *reg;
        }
    }
    throw std::runtime_error { "unable to get a free register" };
}

Register& StackMachine::assignRegisterTo(Value& symbol) {
    Register& reg = get64BitRegister();
    *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value& symbol, Register& registerToExclude) {
    Register& reg = get64BitRegisterExcluding(registerToExclude);
    *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

} /* namespace code_generator */
