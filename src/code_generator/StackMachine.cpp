#include "StackMachine.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "code_generator.h"
#include "InstructionSet.h"

namespace {
const int VARIABLE_SIZE = 4;
}

namespace code_generator {

StackMachine::StackMachine(std::ostream* ostream, std::unique_ptr<InstructionSet> instructions) :
        ostream { ostream },
        instructions { std::move(instructions) },
        stackPointer { "esp" },
        basePointer { "ebp" },
        ioRegisterName { "ecx" },
        retrievalRegisterName { "eax" }
{
    generalPurposeRegisters.insert(std::make_pair("eax", Register { "eax" }));
    generalPurposeRegisters.insert(std::make_pair("ebx", Register { "ebx" }));
    generalPurposeRegisters.insert(std::make_pair("ecx", Register { "ecx" }));
    generalPurposeRegisters.insert(std::make_pair("edx", Register { "edx" }));

}

void StackMachine::startProcedure(std::string procedureName) {
    emptyGeneralPurposeRegisters();
    if (procedureName == "main") {
        main = true;
        *ostream << instructions->mainProcedure();
    } else {
        *ostream << instructions->label(procedureName);
    }
    *ostream << "\t" << instructions->push(basePointer);
    *ostream << "\t" << instructions->mov(stackPointer, basePointer);
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    main = false;
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

void StackMachine::allocateStack(std::vector<Value> values) {
    storeGeneralPurposeRegisterValues();
    *ostream << "\t" << instructions->sub(stackPointer, values.size() * VARIABLE_SIZE);
    for (auto value : values) {
        scopeValues.insert(std::make_pair(value.getName(), value));
    }
}

void StackMachine::deallocateStack() {
    *ostream << "\t" << instructions->add(stackPointer, scopeValues.size() * VARIABLE_SIZE);
    scopeValues.clear();
}

void StackMachine::storeGeneralPurposeRegisterValues() {
    for (auto& reg : generalPurposeRegisters) {
        storeRegisterValue(reg.second);
    }
}

void StackMachine::freeIOregister() {
    storeRegisterValue(generalPurposeRegisters.at(ioRegisterName));
}

void StackMachine::callInputProcedure() {
    *ostream << "\t" << instructions->call("___input");
}

void StackMachine::callOutputProcedure() {
    *ostream << "\t" << instructions->call("___output");
}

void StackMachine::storeIOregisterIn(std::string symbolName) {
    auto& symbol = scopeValues.at(symbolName);
    auto& ioRegister = generalPurposeRegisters.at(ioRegisterName);
    *ostream << "\t" << instructions->mov(ioRegister, memoryBaseRegister(symbol), memoryOffset(symbol));
    ioRegister.assign(&symbol);
}

void StackMachine::assignIOregisterTo(std::string symbolName) {
    auto& symbol = scopeValues.at(symbolName);
    auto& ioRegister = generalPurposeRegisters.at(ioRegisterName);
    if (symbol.isStored()) {
        *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), ioRegister);
    } else {
        Register& valueRegister = generalPurposeRegisters.at(symbol.getAssignedRegisterName());
        *ostream << "\t" << instructions->mov(valueRegister, ioRegister);
        valueRegister.free();
    }
    ioRegister.assign(&symbol);
}

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto& leftSymbol = scopeValues.at(leftSymbolName);
    auto& rightSymbol = scopeValues.at(rightSymbolName);

    if (leftSymbol.isStored() && rightSymbol.isStored()) {
        Register& rightSymbolRegister = assignRegisterTo(rightSymbol);
        *ostream << "\t" << instructions->cmp(
                memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol),
                rightSymbolRegister);
    } else if (leftSymbol.isStored()) {
        *ostream << "\t" << instructions->cmp(
                memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol),
                generalPurposeRegisters.at(rightSymbol.getAssignedRegisterName()));
    } else if (rightSymbol.isStored()) {
        *ostream << "\t" << instructions->cmp(
                generalPurposeRegisters.at(leftSymbol.getAssignedRegisterName()),
                memoryBaseRegister(rightSymbol), memoryOffset(rightSymbol));
    } else {
        *ostream << "\t" << instructions->cmp(
                generalPurposeRegisters.at(leftSymbol.getAssignedRegisterName()),
                generalPurposeRegisters.at(rightSymbol.getAssignedRegisterName()));
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto& symbol = scopeValues.at(symbolName);
    if (symbol.isStored()) {
        *ostream << "\t" << instructions->cmp(memoryBaseRegister(symbol), memoryOffset(symbol), 0);
    } else {
        *ostream << "\t" << instructions->cmp(generalPurposeRegisters.at(symbol.getAssignedRegisterName()), 0);
    }
}

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    storeInMemory(operand);
    Register& resultRegister = getRegister();
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
    Register& resultRegister = getRegisterExcluding(operand.getAssignedRegisterName());
    *ostream << "\t" << instructions->mov(generalPurposeRegisters.at(operand.getAssignedRegisterName()), 0, resultRegister);
    scopeValues.at(resultName).assignRegister(resultRegister.getName());

    Register& lvalueRegister = getRegister();
    *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), lvalueRegister);
    scopeValues.at(lvalueName).assignRegister(lvalueRegister.getName());
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        Register& resultRegister = getRegister();
        *ostream << "\t" << instructions->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
        *ostream << "\t" << instructions->negate(resultRegister);
        *ostream << "\t" << instructions->add(resultRegister, 1); // add dword?
        resultRegister.assign(&scopeValues.at(resultName));
    } else {
        Register& operandRegister = generalPurposeRegisters.at(operand.getAssignedRegisterName());
        Register& resultRegister = getRegisterExcluding(operand.getAssignedRegisterName());
        *ostream << "\t" << instructions->mov(operandRegister, resultRegister);
        *ostream << "\t" << instructions->negate(resultRegister);
        *ostream << "\t" << instructions->add(resultRegister, 1); // add dword?
        resultRegister.assign(&scopeValues.at(resultName));
    }
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    if (operand.isStored() && result.isStored()) {
        Register& resultRegister = assignRegisterTo(result);
        *ostream << "\t" << instructions->mov(
                memoryBaseRegister(operand), memoryOffset(operand),
                resultRegister);
    } else if (operand.isStored()) {
        *ostream << "\t" << instructions->mov(
                memoryBaseRegister(operand), memoryOffset(operand),
                generalPurposeRegisters.at(result.getAssignedRegisterName()));
    } else if (result.isStored()) {
        *ostream << "\t" << instructions->mov(
                generalPurposeRegisters.at(operand.getAssignedRegisterName()),
                memoryBaseRegister(result), memoryOffset(result));
    } else {
        *ostream << "\t" << instructions->mov(
                generalPurposeRegisters.at(operand.getAssignedRegisterName()),
                generalPurposeRegisters.at(result.getAssignedRegisterName()));
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto& result = scopeValues.at(resultName);
    if (result.isStored()) {
        *ostream << "\t" << instructions->mov(constant, memoryBaseRegister(result), memoryOffset(result)); // mov dword?
    } else {
        *ostream << "\t" << instructions->mov(constant, generalPurposeRegisters.at(result.getAssignedRegisterName())); // mov dword?
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto& operand = scopeValues.at(operandName);
    auto& result = scopeValues.at(resultName);

    Register& operandRegister = operand.isStored() ? assignRegisterTo(operand) : generalPurposeRegisters.at(operand.getAssignedRegisterName());
    Register& resultRegister =
            result.isStored() ? assignRegisterExcluding(result, operandRegister.getName()) : generalPurposeRegisters.at(result.getAssignedRegisterName());
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
        argumentOffset += VARIABLE_SIZE;
    }
    argumentNames.clear();
    *ostream << "\t" << instructions->call(procedureName);
    *ostream << "\t" << instructions->add(stackPointer, argumentOffset); // ? add esp, byte " << argumentOffset
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (main) {
        // TODO: return value from main
        *ostream << "\t" << instructions->mov("1", generalPurposeRegisters.at(retrievalRegisterName));
        *ostream << "\t" << instructions->interrupt("0x80");
        *ostream << "\t" << instructions->ret() << "\n";
    } else {
        /* std::string regName = arg->getValue();
         Register *reg = getRegByName(regName);
         if (reg != NULL && reg != eax)
         outfile << "\tmov " << eax->getName() << ", " << reg->getName() << endl;
         else
         outfile << "\tmov " << eax->getName() << ", dword " << getMemoryAddress(arg) << endl;
         outfile << "\tmov esp, ebp\n" << "\tpop ebp\n";
         outfile << "\tret\n\n";
         ebx->free();
         ecx->free();
         edx->free();*/
    }
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value& returnSymbol = scopeValues.at(returnSymbolName);
    *ostream << "\t" << instructions->mov(generalPurposeRegisters.at(retrievalRegisterName), memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol));
}

void StackMachine::pushProcedureArgument(Value& symbolToPush, int argumentOffset) {
    if (symbolToPush.isStored()) {
        Register& reg = getRegister();
        *ostream << "\t" << instructions->mov(
                memoryBaseRegister(symbolToPush),
                symbolToPush.isFunctionArgument() ? memoryOffset(symbolToPush) : memoryOffset(symbolToPush) + argumentOffset,
                reg);
        *ostream << "\t" << instructions->push(reg);
    } else {
        *ostream << "\t" << instructions->push(generalPurposeRegisters.at(symbolToPush.getAssignedRegisterName()));
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
        reg.second.free();
    }
}

void StackMachine::storeInMemory(Value& symbol) {
    if (!symbol.isStored()) {
        storeRegisterValue(generalPurposeRegisters.at(symbol.getAssignedRegisterName()));
    }
}

int StackMachine::memoryOffset(const Value& symbol) const {
    if (symbol.isFunctionArgument()) {
        return (symbol.getIndex() + 2) * VARIABLE_SIZE;
    }
    return symbol.getIndex() * VARIABLE_SIZE;
}

const Register& StackMachine::memoryBaseRegister(const Value& symbol) const {
    if (symbol.isFunctionArgument()) {
        return basePointer;
    }
    return stackPointer;
}

Register& StackMachine::getRegister() {
    for (auto& reg : generalPurposeRegisters) {
        if (!reg.second.containsUnstoredValue()) {
            return reg.second;
        }
    }
    Register& reg = generalPurposeRegisters.begin()->second;
    storeRegisterValue(reg);
    return reg;
}

Register& StackMachine::getRegisterExcluding(std::string registerName) {
    for (auto& reg : generalPurposeRegisters) {
        if (!reg.second.containsUnstoredValue()) {
            return reg.second;
        }
    }
    for (auto& reg : generalPurposeRegisters) {
        if (reg.first != registerName) {
            storeRegisterValue(reg.second);
            return reg.second;
        }
    }
    throw std::runtime_error { "unable to get a free register" };
}

Register& StackMachine::assignRegisterTo(Value& symbol) {
    Register& reg = getRegister();
    *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value& symbol, std::string registerName) {
    Register& reg = getRegisterExcluding(registerName);
    *ostream << "\t" << instructions->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

} /* namespace code_generator */
