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

void StackMachine::generatePreamble(std::map<std::string, std::string> constants) {
    assembly.raw(instructionSet->preamble(constants));
}

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments) {

    emptyGeneralPurposeRegisters();
    assembly.label(instructionSet->label(procedureName));
    assembly << instructionSet->push(registers->getBasePointer());
    assembly << instructionSet->mov(registers->getStackPointer(), registers->getBasePointer());

    for (auto &value : values) {
        scopeValues.insert(std::make_pair(value.getName(), value));
    }
    std::size_t integerArgumentRegisterIndex{0};
    std::size_t localIndex{scopeValues.size()};
    int argumentIndex{0};
    for (auto &argument : arguments) {
        if (argument.getType() == Type::INTEGRAL && integerArgumentRegisterIndex < registers->getIntegerArgumentRegisters().size()) {
            Value registerArgument{argument.getName(), static_cast<int>(localIndex), argument.getType(), argument.getSizeInBytes()};
            scopeValues.insert(std::make_pair(argument.getName(), registerArgument));
            registers->getIntegerArgumentRegisters()[integerArgumentRegisterIndex]->assign(&scopeValues.at(argument.getName()));
            ++integerArgumentRegisterIndex;
            ++localIndex;
        } else {
            Value stackArgument{argument.getName(), argumentIndex, argument.getType(), argument.getSizeInBytes(), true};
            scopeValues.insert(std::make_pair(argument.getName(), stackArgument));
            ++argumentIndex;
        }
    }
    localVariableStackSize = 0;
    int savedRegistersStack = registers->getCalleeSavedRegisters().size() * MACHINE_WORD_SIZE;
    localVariableStackSize = (scopeValues.size() - argumentIndex) * MACHINE_WORD_SIZE;
    int stackSize = savedRegistersStack + localVariableStackSize;
    if (stackSize % STACK_ALIGNMENT) {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize + MACHINE_WORD_SIZE);
    } else {
        assembly << instructionSet->sub(registers->getStackPointer(), localVariableStackSize);
    }

    pushCalleeSavedRegisters();
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeValues.clear();
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
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::spillCallerSavedRegisters() {
    for (auto &reg : registers->getCallerSavedRegisters()) {
        storeRegisterValue(*reg);
    }
}

void StackMachine::callInputProcedure(std::string symbolName) {
    spillGeneralPurposeRegisters();
    auto &operand = scopeValues.at(symbolName);
    Register* rsi = registers->getIntegerArgumentRegisters().at(1);
    assembly << instructionSet->mov(memoryBaseRegister(operand), *rsi);
    int offset = memoryOffset(operand);
    if (offset) {
        assembly << instructionSet->add(*rsi, offset);
    }
    assembly << instructionSet->mov("sfmt", *registers->getIntegerArgumentRegisters().at(0));
    assembly << instructionSet->xor_(registers->getRetrievalRegister(), registers->getRetrievalRegister());
    assembly << instructionSet->call("scanf");
}

void StackMachine::assignRegisterToSymbol(Register &reg, Value &symbol) {
    if (symbol.isStored()) {
        storeRegisterValue(reg);
        assembly << instructionSet->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    } else if (&reg != &symbol.getAssignedRegister()) {
        storeRegisterValue(reg);
        Register &valueRegister = symbol.getAssignedRegister();
        storeRegisterValue(valueRegister);
        assembly << instructionSet->mov(valueRegister, reg);
    }
}

void StackMachine::compare(std::string leftSymbolName, std::string rightSymbolName) {
    auto &leftSymbol = scopeValues.at(leftSymbolName);
    auto &rightSymbol = scopeValues.at(rightSymbolName);

    if (leftSymbol.isStored() && rightSymbol.isStored()) {
        Register &rightSymbolRegister = assignRegisterTo(rightSymbol);
        assembly << instructionSet->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbolRegister);
    } else if (leftSymbol.isStored()) {
        assembly << instructionSet->cmp(memoryBaseRegister(leftSymbol), memoryOffset(leftSymbol), rightSymbol.getAssignedRegister());
    } else if (rightSymbol.isStored()) {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), memoryBaseRegister(rightSymbol), memoryOffset(rightSymbol));
    } else {
        assembly << instructionSet->cmp(leftSymbol.getAssignedRegister(), rightSymbol.getAssignedRegister());
    }
}

void StackMachine::zeroCompare(std::string symbolName) {
    auto &symbol = scopeValues.at(symbolName);
    if (symbol.isStored()) {
        assembly << instructionSet->cmp(memoryBaseRegister(symbol), memoryOffset(symbol), 0);
    } else {
        assembly << instructionSet->cmp(symbol.getAssignedRegister(), 0);
    }
}

void StackMachine::addressOf(std::string operandName, std::string resultName) {
    auto &operand = scopeValues.at(operandName);
    storeInMemory(operand);
    Register &resultRegister = get64BitRegister();
    assembly << instructionSet->mov(memoryBaseRegister(operand), resultRegister);
    int offset = memoryOffset(operand);
    if (offset) {
        assembly << instructionSet->add(resultRegister, offset);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::dereference(std::string operandName, std::string lvalueName, std::string resultName) {
    auto &operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assignRegisterTo(operand);
    }
    Register &resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
    assembly << instructionSet->mov(operand.getAssignedRegister(), 0, resultRegister);
    resultRegister.assign(&scopeValues.at(resultName));

    Register &lvalueRegister = get64BitRegister();
    assembly << instructionSet->mov(memoryBaseRegister(operand), memoryOffset(operand), lvalueRegister);
    lvalueRegister.assign(&scopeValues.at(lvalueName));
}

void StackMachine::unaryMinus(std::string operandName, std::string resultName) {
    auto &operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        Register &resultRegister = get64BitRegister();
        assembly << instructionSet->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
        assembly << instructionSet->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    } else {
        Register &operandRegister = operand.getAssignedRegister();
        Register &resultRegister = get64BitRegisterExcluding(operand.getAssignedRegister());
        assembly << instructionSet->mov(operandRegister, resultRegister);
        assembly << instructionSet->neg(resultRegister);
        resultRegister.assign(&scopeValues.at(resultName));
    }
}

void StackMachine::assign(std::string operandName, std::string resultName) {
    auto &operand = scopeValues.at(operandName);
    auto &result = scopeValues.at(resultName);

    if (operand.isStored() && result.isStored()) {
        Register &resultRegister = assignRegisterTo(result);
        assembly << instructionSet->mov(memoryBaseRegister(operand), memoryOffset(operand), resultRegister);
    } else if (operand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(operand), memoryOffset(operand), result.getAssignedRegister());
    } else if (result.isStored()) {
        assembly << instructionSet->mov(operand.getAssignedRegister(), memoryBaseRegister(result), memoryOffset(result));
    } else {
        assembly << instructionSet->mov(operand.getAssignedRegister(), result.getAssignedRegister());
    }
}

void StackMachine::assignConstant(std::string constant, std::string resultName) {
    auto &result = scopeValues.at(resultName);
    if (result.isStored()) {
        assembly << instructionSet->mov(constant, memoryBaseRegister(result), memoryOffset(result)); // mov dword?
    } else {
        assembly << instructionSet->mov(constant, result.getAssignedRegister()); // mov dword?
    }
}

void StackMachine::lvalueAssign(std::string operandName, std::string resultName) {
    auto &operand = scopeValues.at(operandName);
    auto &result = scopeValues.at(resultName);

    Register &operandRegister = operand.isStored() ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register &resultRegister = result.isStored() ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    assembly << instructionSet->mov(operandRegister, resultRegister, 0);
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
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();
    int argumentOffset{0};
    for (auto argument : stackArguments) {
        pushProcedureArgument(*argument, argumentOffset);
        argumentOffset += MACHINE_WORD_SIZE;
    }
    integerArguments.clear();
    stackArguments.clear();
    //auto &retrievalRegister = registers->getRetrievalRegister();
    //assembly << instructionSet->xor_(retrievalRegister, retrievalRegister);
    assembly << instructionSet->call(procedureName);
    if (argumentOffset) {
        assembly << instructionSet->add(registers->getStackPointer(), argumentOffset);
    }
}

void StackMachine::pushProcedureArgument(Value &symbolToPush, int argumentOffset) {
    if (symbolToPush.isStored()) {
        Register &reg = get64BitRegister();
        assembly << instructionSet->mov(memoryBaseRegister(symbolToPush),
                                        symbolToPush.isFunctionArgument() ? memoryOffset(symbolToPush) : memoryOffset(symbolToPush) + argumentOffset, reg);
        assembly << instructionSet->push(reg);
    } else {
        assembly << instructionSet->push(symbolToPush.getAssignedRegister());
    }
}

void StackMachine::returnFromProcedure(std::string returnSymbolName) {
    if (!returnSymbolName.empty()) {
        Value &returnSymbol = scopeValues.at(returnSymbolName);
        if (returnSymbol.isStored()) {
            assembly << instructionSet->mov(memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol), registers->getRetrievalRegister());
        } else if (&registers->getRetrievalRegister() != &returnSymbol.getAssignedRegister()) {
            assembly << instructionSet->mov(returnSymbol.getAssignedRegister(), registers->getRetrievalRegister());
        }
    }
    popCalleeSavedRegisters();
    assembly << instructionSet->leave();
    assembly << instructionSet->ret();
}

void StackMachine::retrieveProcedureReturnValue(std::string returnSymbolName) {
    Value &returnSymbol = scopeValues.at(returnSymbolName);
    assembly << instructionSet->mov(registers->getRetrievalRegister(), memoryBaseRegister(returnSymbol), memoryOffset(returnSymbol));
}

void StackMachine::xorCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Register &resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->xor_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->xor_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructionSet->xor_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->xor_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::orCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Register &resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->or_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->or_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructionSet->or_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->or_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::andCommand(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Register &resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->and_(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->and_(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
        assembly << instructionSet->and_(leftOperand.getAssignedRegister(), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->and_(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::add(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Register &resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->add(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->add(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->add(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->add(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::sub(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Register &resultRegister = get64BitRegister();

    if (leftOperand.isStored() && rightOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->sub(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
        assembly << instructionSet->sub(rightOperand.getAssignedRegister(), resultRegister);
    } else if (rightOperand.isStored()) {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->sub(memoryBaseRegister(rightOperand), memoryOffset(rightOperand), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
        assembly << instructionSet->sub(rightOperand.getAssignedRegister(), resultRegister);
    }
    resultRegister.assign(&scopeValues.at(resultName));
}

void StackMachine::mul(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Value &result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"multiplication of non integers is not implemented"};
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    if (rightOperand.isStored()) {
        assembly << instructionSet->imul(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructionSet->imul(rightOperand.getAssignedRegister());
    }
    registers->getMultiplicationRegister().assign(&result);
}

void StackMachine::div(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Value &result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"division of non integer types is not implemented"};
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
        assembly << instructionSet->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    registers->getMultiplicationRegister().assign(&result);
}

void StackMachine::mod(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    Value &leftOperand = scopeValues.at(leftOperandName);
    Value &rightOperand = scopeValues.at(rightOperandName);
    Value &result = scopeValues.at(resultName);

    if (result.getType() != Type::INTEGRAL) {
        throw std::runtime_error{"modular division of non integer types is not implemented"};
    }

    if (leftOperand.isStored()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), registers->getMultiplicationRegister());
    } else if (&leftOperand.getAssignedRegister() != &registers->getMultiplicationRegister()) {
        storeRegisterValue(registers->getMultiplicationRegister());
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), registers->getMultiplicationRegister());
    }
    storeRegisterValue(registers->getRemainderRegister());
    assembly << instructionSet->xor_(registers->getRemainderRegister(), registers->getRemainderRegister());
    if (rightOperand.isStored()) {
        assembly << instructionSet->idiv(memoryBaseRegister(rightOperand), memoryOffset(rightOperand));
    } else {
        assembly << instructionSet->idiv(rightOperand.getAssignedRegister());
    }
    registers->getRemainderRegister().assign(&result);
}

void StackMachine::inc(std::string operandName) {
    Value &operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assembly << instructionSet->inc(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        assembly << instructionSet->inc(operand.getAssignedRegister());
    }
}

void StackMachine::dec(std::string operandName) {
    Value &operand = scopeValues.at(operandName);
    if (operand.isStored()) {
        assembly << instructionSet->dec(memoryBaseRegister(operand), memoryOffset(operand));
    } else {
        assembly << instructionSet->dec(operand.getAssignedRegister());
    }
}

void StackMachine::shl(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    // shl r|m imm|cl
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = scopeValues.at(rightOperandName);
    assignRegisterToSymbol(counterRegister, rightOperand);

    Register& resultRegister = get64BitRegister();
    Value& leftOperand = scopeValues.at(leftOperandName);
    if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    // shl resultRegister cl ; the shift argument is put in counter register - rcx
    assembly << instructionSet->shl(resultRegister);
    Value& result = scopeValues.at(resultName);
    resultRegister.assign(&result);
}

void StackMachine::shr(std::string leftOperandName, std::string rightOperandName, std::string resultName) {
    // shr r|m imm|cl
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = scopeValues.at(rightOperandName);
    assignRegisterToSymbol(counterRegister, rightOperand);

    Register& resultRegister = get64BitRegister();
    Value& leftOperand = scopeValues.at(leftOperandName);
    if (leftOperand.isStored()) {
        assembly << instructionSet->mov(memoryBaseRegister(leftOperand), memoryOffset(leftOperand), resultRegister);
    } else {
        assembly << instructionSet->mov(leftOperand.getAssignedRegister(), resultRegister);
    }
    // shr resultRegister cl ; the shift argument is put in counter register - rcx
    assembly << instructionSet->shr(resultRegister);
    Value& result = scopeValues.at(resultName);
    resultRegister.assign(&result);
}

void StackMachine::storeRegisterValue(Register &reg) {
    if (reg.containsUnstoredValue()) {
        assembly << instructionSet->mov(reg, memoryBaseRegister(*reg.getValue()), memoryOffset(*reg.getValue()));
        reg.free();
    }
}

void StackMachine::emptyGeneralPurposeRegisters() {
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        reg->free();
    }
}

void StackMachine::pushCalleeSavedRegisters() { pushRegisters(registers->getCalleeSavedRegisters(), calleeSavedRegisters); }

void StackMachine::popCalleeSavedRegisters() { popRegisters(calleeSavedRegisters); }

void StackMachine::pushDirtyRegisters(std::vector<Register *> source, std::vector<Register *> &destination) {
    for (auto &reg : source) {
        if (reg->containsUnstoredValue()) {
            pushRegister(*reg, destination);
        }
    }
}

void StackMachine::pushRegisters(std::vector<Register *> source, std::vector<Register *> &destination) {
    for (auto &reg : source) {
        pushRegister(*reg, destination);
    }
}

void StackMachine::popRegisters(std::vector<Register *> registers) {
    for (auto &reg : registers) {
        assembly << instructionSet->pop(*reg);
    }
}

void StackMachine::pushRegister(Register &reg, std::vector<Register *> &registers) {
    registers.insert(registers.begin(), &reg);
    assembly << instructionSet->push(reg);
}

void StackMachine::storeInMemory(Value &symbol) {
    if (!symbol.isStored()) {
        storeRegisterValue(symbol.getAssignedRegister());
    }
}

int StackMachine::memoryOffset(const Value &symbol) const {
    if (symbol.isFunctionArgument()) {
        return (symbol.getIndex() + 2) * MACHINE_WORD_SIZE;
    }
    return symbol.getIndex() * MACHINE_WORD_SIZE + calleeSavedRegisters.size() * MACHINE_WORD_SIZE;
}

const Register &StackMachine::memoryBaseRegister(const Value &symbol) const {
    if (symbol.isFunctionArgument()) {
        return registers->getBasePointer();
    }
    return registers->getStackPointer();
}

Register &StackMachine::get64BitRegister() {
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    Register &reg = **registers->getGeneralPurposeRegisters().begin();
    storeRegisterValue(reg);
    return reg;
}

Register& StackMachine::get64BitRegisterExcluding(Register &registerToExclude) {
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
        if (!reg->containsUnstoredValue()) {
            return *reg;
        }
    }
    for (auto &reg : registers->getGeneralPurposeRegisters()) {
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

Register& StackMachine::assignRegisterTo(Value &symbol) {
    Register &reg = get64BitRegister();
    assembly << instructionSet->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

Register& StackMachine::assignRegisterExcluding(Value &symbol, Register &registerToExclude) {
    Register& reg = get64BitRegisterExcluding(registerToExclude);
    assembly << instructionSet->mov(memoryBaseRegister(symbol), memoryOffset(symbol), reg);
    reg.assign(&symbol);
    return reg;
}

void StackMachine::setScope(std::vector<Value> variables) {
    for (auto &var : variables) {
        scopeValues.insert(std::make_pair(var.getName(), var));
    }
}

} // namespace codegen

