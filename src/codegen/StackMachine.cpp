#include "StackMachine.h"

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

void StackMachine::startProcedure(std::string procedureName, std::vector<Value> values, std::vector<Value> arguments) {

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
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
    std::size_t integerArgumentRegisterIndex{0};
    std::vector<std::string> floatingRegArgNames;
    int localIndex{nextLocalWord};
    int argumentIndex{0};
    for (auto& argument : arguments) {
        if (argument.getType() == Type::FLOATING && floatingRegArgNames.size() < 8) {
            // Double/float arrive in xmmN; give a spill home and copy after frame setup.
            Value floatArgument{argument.getName(), localIndex, argument.getType(), argument.getSizeInBytes()};
            scopeValues.insert({argument.getName(), floatArgument});
            floatingRegArgNames.push_back(argument.getName());
            localIndex += wordSlots(floatArgument);
        } else if (argument.getType() == Type::INTEGRAL
                && integerArgumentRegisterIndex < registers->getIntegerArgumentRegisters().size()) {
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

    // Copy incoming floating register args (xmm0..) into their spill homes.
    for (std::size_t i = 0; i < floatingRegArgNames.size() && i < 8; ++i) {
        auto& home = resolve(floatingRegArgNames[i]);
        Register& tmp = registers->getRetrievalRegister();
        xmmToGpr(static_cast<int>(i), tmp, isSseFloat32(home));
        emitStore(tmp, home);
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

void StackMachine::procedureArgument(std::string argumentName) {
    auto argument = &resolve(argumentName);
    if (argument->getType() == Type::FLOATING) {
        if (floatingArguments.size() < 8) {
            floatingArguments.push_back(argument);
        } else {
            stackArguments.insert(stackArguments.begin(), argument);
        }
        return;
    }
    if (integerArguments.size() < registers->getIntegerArgumentRegisters().size()) {
        integerArguments.push_back(argument);
    } else {
        stackArguments.insert(stackArguments.begin(), argument);
    }
}

int StackMachine::emitCallArguments() {
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    for (std::size_t i = 0; i < integerArguments.size(); ++i) {
        assignRegisterToSymbol(*integerArgRegs[i], *integerArguments[i]);
    }
    storeRegisterValue(registers->getRetrievalRegister());
    spillCallerSavedRegisters();
    int argumentOffset { 0 };
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

    // Floating args: IEEE bits into xmm0..xmm7 only (SysV).
    for (std::size_t fi = 0; fi < floatingArguments.size(); ++fi) {
        Value& fv = *floatingArguments[fi];
        Register& tmp = get64BitRegisterExcluding(*integerArgRegs[0]);
        if (residesInMemory(fv)) {
            Address home = addressOf(fv);
            if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer && argumentOffset) {
                home = Address::frame(FrameBase::StackPointer,
                        home.offsetBytes() + argumentOffset, home.sizeBytes());
            }
            assembly << instructionSet->mov(memoryOperand(home), tmp);
        } else {
            Register& cur = fv.getAssignedRegister();
            if (&cur != &tmp) {
                assembly << instructionSet->mov(cur, tmp);
            }
        }
        gprToXmm(tmp, static_cast<int>(fi), isSseFloat32(fv));
    }

    const std::size_t xmmCount = floatingArguments.size();
    integerArguments.clear();
    floatingArguments.clear();
    stackArguments.clear();
    // AL = number of vector registers used (System V variadic).
    Register& rax = registers->getMultiplicationRegister();
    if (xmmCount == 0) {
        assembly << instructionSet->xor_(rax, rax);
    } else {
        assembly << instructionSet->mov(std::to_string(xmmCount), rax);
    }
    return argumentOffset;
}

void StackMachine::callProcedure(std::string procedureName) {
    int argumentOffset = emitCallArguments();
    if (isDefinedProcedure(procedureName)) {
        assembly << instructionSet->call(procedureName);
    } else {
        assembly << instructionSet->callPlt(procedureName);
    }
    if (argumentOffset) {
        assembly << instructionSet->add(registers->getStackPointer(), argumentOffset);
    }
}

void StackMachine::callProcedureIndirect(std::string targetSymbolName) {
    int argumentOffset = emitCallArguments();

    // Load callee into r10 after arg setup (caller-saved, not an integer arg reg).
    auto& targetValue = resolve(targetSymbolName);
    Register& targetReg = registers->getIndirectCallTargetRegister();

    if (!residesInMemory(targetValue)) {
        Register& current = targetValue.getAssignedRegister();
        if (&current != &targetReg) {
            assembly << instructionSet->mov(current, targetReg);
        }
    } else {
        Address home = addressOf(targetValue);
        if (!home.isGlobal() && home.frameBase() == FrameBase::StackPointer && argumentOffset) {
            Address adjusted = Address::frame(FrameBase::StackPointer,
                    home.offsetBytes() + argumentOffset, home.sizeBytes());
            assembly << instructionSet->mov(memoryOperand(adjusted), targetReg);
        } else {
            emitLoad(targetValue, targetReg);
        }
    }

    assembly << instructionSet->callIndirect(targetReg);
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
        if (returnSymbol.getType() == Type::FLOATING) {
            // System V: scalar float/double returns in xmm0.
            Register& tmp = registers->getRetrievalRegister();
            if (residesInMemory(returnSymbol)) {
                emitLoad(returnSymbol, tmp);
            } else if (&tmp != &returnSymbol.getAssignedRegister()) {
                assembly << instructionSet->mov(returnSymbol.getAssignedRegister(), tmp);
            }
            gprToXmm(tmp, 0, isSseFloat32(returnSymbol));
        } else if (residesInMemory(returnSymbol)) {
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
    Register& ret = registers->getRetrievalRegister();
    // System V: floating returns arrive in xmm0.
    if (returnSymbol.getType() == Type::FLOATING) {
        xmmToGpr(0, ret, isSseFloat32(returnSymbol));
    }
    emitStore(ret, returnSymbol);
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

