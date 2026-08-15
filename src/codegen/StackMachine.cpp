#include "StackMachine.h"

#include "SysVCallConv.h"
#include "types/ObjectAbi.h"

#include <cassert>
#include <stdexcept>

namespace {
const int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;

bool nativeMoveSize(int n) {
    return n == 1 || n == 2 || n == 4 || n == 8;
}
} // namespace

namespace codegen {

StackMachine::StackMachine(std::ostream *ostream, InstructionSet& instructionSet,
        Amd64Registers& registers, const IrStringTable& strings) :
        assembly{ostream},
        instructionSet{&instructionSet},
        registers{&registers},
        strings_{strings} {}

const std::string& StackMachine::text(int id) const {
    static const std::string empty;
    if (id < 0) {
        return empty;
    }
    return strings_.get(id);
}

void StackMachine::put(std::deque<Value>& storage, std::vector<Value*>& byId, Value value) {
    const int id = value.id();
    if (id < 0) {
        throw std::logic_error { "StackMachine::put: Value has no intern id" };
    }
    if (id >= static_cast<int>(byId.size())) {
        byId.resize(static_cast<std::size_t>(id) + 1, nullptr);
    }
    if (byId[static_cast<std::size_t>(id)] != nullptr) {
        throw std::logic_error { "StackMachine::put: duplicate intern id `" + strings_.get(id) + "`" };
    }
    storage.push_back(std::move(value));
    byId[static_cast<std::size_t>(id)] = &storage.back();
}

void StackMachine::generatePreamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalSymbols) {
    assembly.raw(instructionSet->preamble(constants, globalVariables, externalSymbols));
    for (const auto& global : globalVariables) {
        const int id = strings_.require(global.name);
        globalHomes.emplace(id, Address::globalLabel(global.name, global.sizeInBytes));
        // resolve() shell only; home is globalHomes, never register-cached.
        put(globalStorage, globalById, global.toValue(strings_));
    }
}

void StackMachine::finishInstruction() {
    for (auto& reg : registers->getGeneralPurposeRegisters()) {
        if (!reg->containsUnstoredValue()) {
            continue;
        }
        Value* value = reg->getValue();
        const int lastUse = value->getLastUseOrdinal();
        if (lastUse >= 0 && lastUse <= instructionOrdinal) {
            reg->free();
        }
    }
    ++instructionOrdinal;
}

void StackMachine::registerDefinedProcedure(int procedureName) {
    definedProcedures.insert(procedureName);
}

bool StackMachine::isDefinedProcedure(int name) const {
    return definedProcedures.count(name) > 0;
}

void StackMachine::startProcedure(const Procedure& procedure) {
    const std::string& procedureName = text(procedure.name);
    const std::vector<Value>& values = procedure.frame.locals;
    const std::vector<Value>& arguments = procedure.frame.arguments;
    const bool memoryReturn = procedure.memoryReturn;
    const bool variadic = procedure.variadic;

    emptyGeneralPurposeRegisters();
    frameHomes.clear();
    sretId_ = kNoSymbol;
    variadicFrame.reset();
    hasFrame_ = true;
    frameLayout_ = {};
    instructionOrdinal = 0;
    const int lastNamedFormal = arguments.empty() ? kNoSymbol : arguments.back().id();
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
        put(scopeStorage, scopeById, value);
        const int end = value.getIndex() + wordSlots(value);
        if (end > nextLocalWord) {
            nextLocalWord = end;
        }
    }
    SysVArgCounts argCounts;
    const auto& integerArgRegs = registers->getIntegerArgumentRegisters();
    const std::size_t maxIntegerRegs = integerArgRegs.size();
    struct IncomingRegArg {
        int name;
        SysVArgAssignment asgn;
    };
    std::vector<IncomingRegArg> incomingRegArgs;
    int localIndex{nextLocalWord};
    std::vector<const Value*> stackArgs;
    if (memoryReturn) {
        Value sret { procedure.sretId, localIndex, Type::INTEGRAL, MACHINE_WORD_SIZE };
        sretId_ = procedure.sretId;
        put(scopeStorage, scopeById, std::move(sret));
        integerArgRegs[0]->assign(&resolve(sretId_));
        argCounts.integerRegs = 1;
        localIndex += 1;
    }

    const int vaGpSlots = static_cast<int>(procedure.vaGpHomes.size());
    const int vaXmmWordsEach = SYSV_XMM_SAVE_STRIDE / MACHINE_WORD_SIZE;
    const int vaSaveBaseIndex = variadic ? localIndex : -1;
    if (variadic) {
        localIndex += vaGpSlots + static_cast<int>(procedure.vaXmmHomes.size()) * vaXmmWordsEach;
    }

    for (auto& argument : arguments) {
        const SysVArgAssignment asgn = assignSysVArg(argument.getClassification(), argCounts, maxIntegerRegs);
        lastFormalOnStack = asgn.onStack;
        if (asgn.onStack) {
            stackArgs.push_back(&argument);
            continue;
        }
        const int home = type::object_abi::takeAlignedWords(
                localIndex, argument.getClassification().alignBytes, wordSlots(argument));
        Value registerArgument = argument.withIndex(home);
        const int argumentId = registerArgument.id();
        put(scopeStorage, scopeById, std::move(registerArgument));
        if (asgn.count == 1 && asgn.slots[0] == SysVArgSlot::IntegerReg
                && argument.getClassification().gprExtend == type::sysv::GprExtend::None) {
            integerArgRegs[asgn.indices[0]]->assign(&resolve(argumentId));
        } else {
            incomingRegArgs.push_back({ argumentId, asgn });
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
        put(scopeStorage, scopeById, argument);
        registerFrameHome(argument.id(), Address::frame(FrameBase::BasePointer,
                2 * MACHINE_WORD_SIZE + stackLayout.slots[i].offsetBytes, argument.getSizeInBytes()));
    }

    if (variadic) {
        createVaSaveHomes(vaSaveBaseIndex, procedure.vaGpHomes, procedure.vaXmmHomes);
    }

    int savedRegistersStack = registers->getCalleeSavedRegisters().size() * MACHINE_WORD_SIZE;
    frameLayout_ = type::object_abi::frameLayout(localIndex, savedRegistersStack);
    assembly << instructionSet->sub(registers->getStackPointer(), frameLayout_.subBytes);

    pushCalleeSavedRegisters();
    for (auto& value : scopeStorage) {
        if (frameHomes.count(value.id())) {
            continue;
        }
        registerFrameHome(value.id(), spillSlotAddress(value));
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
        dumpVariadicSaveArea(procedure.vaGpHomes, procedure.vaXmmHomes);
        spillGeneralPurposeRegisters();
        emptyGeneralPurposeRegisters();
        variadicFrame = VariadicFrame {
                addressOf(resolve(procedure.vaGpHomes[0])),
                Address::frame(FrameBase::BasePointer, 2 * MACHINE_WORD_SIZE, MACHINE_WORD_SIZE),
                lastNamedFormal,
                lastFormalOnStack,
                sysvNamedGpOffset(argCounts),
                sysvNamedFpOffset(argCounts),
        };
    }

    for (const auto& incoming : incomingRegArgs) {
        bindGprExtended(resolve(incoming.name));
    }
}

void StackMachine::endProcedure() {
    emptyGeneralPurposeRegisters();
    scopeStorage.clear();
    scopeById.clear();
    frameHomes.clear();
    calleeSavedRegisters.clear();
    sretId_ = kNoSymbol;
    variadicFrame.reset();
    hasFrame_ = false;
    frameLayout_ = {};
}

void StackMachine::label(int name) {
    spillGeneralPurposeRegisters();
    assembly.label(instructionSet->label(text(name)));
}

void StackMachine::jump(JumpCondition jumpCondition, int label, bool signedRel) {
    // Spill on every outgoing edge. Conditional jumps used to skip this, so a branch
    // to a join label could skip the spill that label() emits only on fall-through —
    // leaving live values (e.g. argument registers) in regs while later code reloads
    // them from unsaved stack slots. Repro: `int f(int a){ if(0); return a; }`.
    spillGeneralPurposeRegisters();
    const std::string& labelName = text(label);
    switch (jumpCondition) {
    case JumpCondition::IF_EQUAL:
        assembly << instructionSet->je(labelName);
        break;
    case JumpCondition::IF_NOT_EQUAL:
        assembly << instructionSet->jne(labelName);
        break;
    case JumpCondition::IF_ABOVE:
        assembly << (signedRel ? instructionSet->jg(labelName) : instructionSet->ja(labelName));
        break;
    case JumpCondition::IF_BELOW:
        assembly << (signedRel ? instructionSet->jl(labelName) : instructionSet->jb(labelName));
        break;
    case JumpCondition::IF_ABOVE_OR_EQUAL:
        assembly << (signedRel ? instructionSet->jge(labelName) : instructionSet->jae(labelName));
        break;
    case JumpCondition::IF_BELOW_OR_EQUAL:
        assembly << (signedRel ? instructionSet->jle(labelName) : instructionSet->jbe(labelName));
        break;
    case JumpCondition::UNCONDITIONAL:
    default:
        assembly << instructionSet->jmp(labelName);
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
    if (symbol.getType() == Type::INTEGRAL) {
        loadPromoted(symbol, dest);
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
    storeObject(source, symbol);
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
    canonicalize(reg, result);
    reg.assign(&result);
}

void StackMachine::compare(int leftSymbolName, int rightSymbolName, bool signedRel) {
    auto& leftSymbol = resolve(leftSymbolName);
    auto& rightSymbol = resolve(rightSymbolName);
    if (tryComplexCompare(leftSymbol, rightSymbol)) {
        return;
    }
    if (isX87Float(leftSymbol) || isX87Float(rightSymbol)) {
        emitX87Compare(leftSymbol, rightSymbol, signedRel);
        return;
    }
    if (tryWideCompare(leftSymbol, rightSymbol, signedRel)) {
        return;
    }

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

    const int width = (promotedBytes(leftSymbol) == 4 && promotedBytes(rightSymbol) == 4) ? 4 : 8;
    Register& leftReg = materialize(leftSymbol);
    Register& rightReg = materializeExcluding(rightSymbol, leftReg);
    assembly << instructionSet->cmp(leftReg, rightReg, width);
}

void StackMachine::zeroCompare(int symbolName) {
    auto& symbol = resolve(symbolName);
    if (tryComplexZeroCompare(symbol)) {
        return;
    }
    if (isX87Float(symbol)) {
        emitX87ZeroCompare(symbol);
        return;
    }
    if (tryWideZeroCompare(symbol)) {
        return;
    }
    Register& reg = materialize(symbol);
    assembly << instructionSet->cmp(reg, 0, promotedBytes(symbol));
}

void StackMachine::addressOf(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    storeInMemory(operand);
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(memoryOperand(operand), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::functionAddress(int functionName, int resultName) {
    Register& resultRegister = get64BitRegister();
    const std::string& name = text(functionName);
    if (isDefinedProcedure(functionName)) {
        assembly << instructionSet->lea(MemoryOperand::global(name), resultRegister);
    } else {
        assembly << instructionSet->loadGot(name, resultRegister);
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::dereference(int operandName, int lvalueName, int resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);
    Register& pointerRegister = materialize(operand);
    if (!nativeMoveSize(result.getSizeInBytes())) {
        copyFromPointer(pointerRegister, result);
    } else {
        Register& resultRegister = get64BitRegisterExcluding(pointerRegister);
        loadPromotedFrom(MemoryOperand::at(pointerRegister, 0), result, resultRegister);
        bindResult(resultRegister, result);
    }

    Register& lvalueRegister = get64BitRegisterExcluding(pointerRegister);
    assembly << instructionSet->mov(pointerRegister, lvalueRegister);
    lvalueRegister.assign(&resolve(lvalueName));
}

void StackMachine::unaryMinus(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    if (tryComplexUnaryMinus(operand, resolve(resultName))) {
        return;
    }
    if (isX87Float(operand)) {
        emitX87UnaryMinus(operand, resolve(resultName));
        return;
    }
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
    Value& result = resolve(resultName);
    if (tryWideUnaryMinus(operand, result)) {
        return;
    }
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    assembly << instructionSet->neg(resultRegister, promotedBytes(result));
    bindResult(resultRegister, result);
}

void StackMachine::bswap(int operandName, int resultName, int widthBytes) {
    auto& operand = resolve(operandName);
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    for (const auto& insn : instructionSet->bswap(resultRegister, widthBytes)) {
        assembly << insn;
    }
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::unaryNot(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    Value& result = resolve(resultName);
    if (tryWideUnaryNot(operand, result)) {
        return;
    }
    Register& resultRegister = get64BitRegister();
    copyToRegister(operand, resultRegister);
    assembly << instructionSet->not_(resultRegister, promotedBytes(result));
    bindResult(resultRegister, result);
}

void StackMachine::widenInteger(int operandName, int resultName, bool signHighWord) {
    Value& operand = resolve(operandName);
    Value& result = resolve(resultName);
    storeInMemory(operand);
    if (type::object_abi::valueWords(result.getSizeInBytes()) <= 1) {
        Register& dest = get64BitRegister();
        loadWord(operand, 0, dest);
        bindResult(dest, result);
        return;
    }
    Register& lo = registers->getRetrievalRegister();
    Register& hi = registers->getRemainderRegister();
    storeRegisterValue(lo);
    storeRegisterValue(hi);
    loadWord(operand, 0, lo);
    if (signHighWord) {
        assembly << instructionSet->cqo();
    } else {
        assembly << instructionSet->xor_(hi, hi);
    }
    storeWord(lo, result, 0);
    storeWord(hi, result, 1);
}

void StackMachine::assign(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    if (tryComplexAssignConvert(operand, result)) {
        return;
    }
    if (tryNumericAssignConvert(operand, result)) {
        return;
    }

    if (type::object_abi::valueWords(operand.getSizeInBytes()) > 1
            || type::object_abi::valueWords(result.getSizeInBytes()) > 1) {
        copyWords(operand, result);
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
    if (!residesInMemory(result)) {
        bindResult(result.getAssignedRegister(), result);
    }
}

void StackMachine::assignConstant(int constant, int resultName, int highWord) {
    auto& result = resolve(resultName);
    if (type::object_abi::valueWords(result.getSizeInBytes()) > 1) {
        Register& lo = get64BitRegister();
        assembly << instructionSet->mov(text(constant), lo);
        storeWord(lo, result, 0);
        Register& hi = get64BitRegisterExcluding(lo);
        assembly << instructionSet->mov(highWord < 0 ? "0" : text(highWord), hi);
        storeWord(hi, result, 1);
        return;
    }
    // Float IEEE bits and large integers exceed signed 32-bit imm to memory; go via register.
    Register& reg = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    assembly << instructionSet->mov(text(constant), reg);
    if (residesInMemory(result)) {
        emitStore(reg, result);
    } else {
        bindResult(reg, result);
    }
}

void StackMachine::assignLabelAddress(int label, int resultName) {
    Register& resultRegister = get64BitRegister();
    assembly << instructionSet->lea(MemoryOperand::global(text(label)), resultRegister);
    bindResult(resultRegister, resolve(resultName));
}

void StackMachine::lvalueAssign(int operandName, int resultName) {
    auto& operand = resolve(operandName);
    auto& result = resolve(resultName);

    const int storeSize = operand.getSizeInBytes();
    if (!nativeMoveSize(storeSize)) {
        Register& ptr = residesInMemory(result) ? assignRegisterTo(result) : result.getAssignedRegister();
        copyToPointer(operand, ptr);
        return;
    }

    Register& operandRegister = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& resultRegister = residesInMemory(result) ? assignRegisterExcluding(result, operandRegister) : result.getAssignedRegister();
    storeObject(operandRegister, MemoryOperand::at(resultRegister, 0), storeSize);
}

void StackMachine::xorCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Xor)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Xor);
}

void StackMachine::orCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Or)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Or);
}

void StackMachine::andCommand(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::And)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::And);
}

bool StackMachine::involvesFloating(const Value& left, const Value& right, const Value& result) const {
    return left.getType() == Type::FLOATING || right.getType() == Type::FLOATING
            || result.getType() == Type::FLOATING;
}

void StackMachine::add(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryComplexBinary(leftOperand, rightOperand, result, X87Op::Add)) {
        return;
    }
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::addss, &InstructionSet::addsd, X87Op::Add);
        return;
    }
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Add)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Add);
}

void StackMachine::sub(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryComplexBinary(leftOperand, rightOperand, result, X87Op::Sub)) {
        return;
    }
    if (involvesFloating(leftOperand, rightOperand, result)) {
        emitFloatingOrX87Binary(leftOperand, rightOperand, result,
                &InstructionSet::subss, &InstructionSet::subsd, X87Op::Sub);
        return;
    }
    if (tryWideIntegerBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub)) {
        return;
    }
    emitGprBinary(leftOperand, rightOperand, result, WideIntegerOp::Sub);
}

void StackMachine::stepLvalue(Value& operand, bool increment, int step) {
    Register& reg = get64BitRegister();
    copyToRegister(operand, reg);
    if (!residesInMemory(operand)) {
        operand.getAssignedRegister().free();
    }
    if (step == 1) {
        assembly << (increment ? instructionSet->inc(reg, promotedBytes(operand))
                               : instructionSet->dec(reg, promotedBytes(operand)));
    } else if (increment) {
        assembly << instructionSet->add(reg, step);
    } else {
        assembly << instructionSet->sub(reg, step);
    }
    bindResult(reg, operand);
}

void StackMachine::inc(int operandName, int step) {
    stepLvalue(resolve(operandName), true, step);
}

void StackMachine::dec(int operandName, int step) {
    stepLvalue(resolve(operandName), false, step);
}

void StackMachine::shiftBy(int leftOperandName, int rightOperandName, int resultName,
        std::string (InstructionSet::*emitShift)(const Register&, int) const) {
    // Count must live in %cl (RCX) and be tracked so the value is not placed in RCX.
    Register& counterRegister = getCounterRegister();
    Value& rightOperand = resolve(rightOperandName);
    copyToRegister(rightOperand, counterRegister);
    if (!addressOf(rightOperand).isGlobal()) {
        if (!residesInMemory(rightOperand)
                && &rightOperand.getAssignedRegister() != &counterRegister) {
            rightOperand.getAssignedRegister().free();
        }
        counterRegister.assign(&rightOperand);
    }

    Value& leftOperand = resolve(leftOperandName);
    Register& resultRegister = get64BitRegisterExcluding(counterRegister);
    copyToRegister(leftOperand, resultRegister);
    assembly << (instructionSet->*emitShift)(resultRegister, promotedBytes(leftOperand));
    Value& result = resolve(resultName);
    bindResult(resultRegister, result);
}

void StackMachine::shl(int leftOperandName, int rightOperandName, int resultName) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    if (tryWideShift(leftOperand, rightOperand, result, WideShiftOp::Left)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName, &InstructionSet::shl);
}

void StackMachine::shr(int leftOperandName, int rightOperandName, int resultName,
        bool arithmetic) {
    Value& leftOperand = resolve(leftOperandName);
    Value& rightOperand = resolve(rightOperandName);
    Value& result = resolve(resultName);
    const WideShiftOp op = arithmetic ? WideShiftOp::ArithmeticRight : WideShiftOp::LogicalRight;
    if (tryWideShift(leftOperand, rightOperand, result, op)) {
        return;
    }
    shiftBy(leftOperandName, rightOperandName, resultName,
            arithmetic ? &InstructionSet::shr : &InstructionSet::lshr);
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

void StackMachine::popCalleeSavedRegisters() {
    if (hasFrame_ && !calleeSavedRegisters.empty()) {
        const int offset = -frameLayout_.subBytes
                - static_cast<int>(calleeSavedRegisters.size()) * MACHINE_WORD_SIZE;
        assembly << instructionSet->lea(
                MemoryOperand::at(registers->getBasePointer(), offset),
                registers->getStackPointer());
    }
    popRegisters(calleeSavedRegisters);
}

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
    if (hasFrame_) {
        return Address::frame(FrameBase::BasePointer,
                -frameLayout_.homeBytes + symbol.getIndex() * MACHINE_WORD_SIZE,
                symbol.getSizeInBytes());
    }
    int offset = symbol.getIndex() * MACHINE_WORD_SIZE
            + static_cast<int>(calleeSavedRegisters.size()) * MACHINE_WORD_SIZE;
    return Address::frame(FrameBase::StackPointer, offset, symbol.getSizeInBytes());
}

void StackMachine::registerFrameHome(int id, Address address) {
    [[maybe_unused]] const bool inserted =
            frameHomes.emplace(id, std::move(address)).second;
    assert(inserted && "duplicate frame home registration");
}

bool StackMachine::residesInMemory(const Value& symbol) const {
    return addressOf(symbol).isGlobal() || symbol.isStored();
}

Address StackMachine::addressOf(const Value& symbol) const {
    const int id = symbol.id();
    auto frame = frameHomes.find(id);
    if (frame != frameHomes.end()) {
        return frame->second;
    }
    auto global = globalHomes.find(id);
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

MemoryOperand StackMachine::memoryOperandAt(const Value& symbol, int byteOffset) const {
    Address home = addressOf(symbol);
    assert(!home.isGlobal() && "memoryOperandAt is frame-only; use loadX87At/storeX87At for globals");
    return MemoryOperand::at(
            home.frameBase() == FrameBase::BasePointer
                    ? registers->getBasePointer() : registers->getStackPointer(),
            home.offsetBytes() + byteOffset);
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
        put(scopeStorage, scopeById, var);
        registerFrameHome(scopeStorage.back().id(), spillSlotAddress(scopeStorage.back()));
    }
}

Value& StackMachine::resolve(int id) {
    if (id >= 0 && id < static_cast<int>(scopeById.size()) && scopeById[static_cast<std::size_t>(id)] != nullptr) {
        return *scopeById[static_cast<std::size_t>(id)];
    }
    if (id >= 0 && id < static_cast<int>(globalById.size()) && globalById[static_cast<std::size_t>(id)] != nullptr) {
        return *globalById[static_cast<std::size_t>(id)];
    }
    throw std::runtime_error { "codegen: no storage for symbol `" + text(id)
            + "` (function designator or missing global?)" };
}

} // namespace codegen

