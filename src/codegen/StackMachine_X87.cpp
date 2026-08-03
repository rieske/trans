#include "StackMachine.h"

#include "InstructionSet.h"

namespace codegen {

bool StackMachine::tryX87Binary(Value& left, Value& right, Value& result, X87Op op) {
    if (!isX87Float(result)) {
        return false;
    }
    emitX87Binary(left, right, result, op);
    return true;
}

void StackMachine::emitX87Binary(Value& left, Value& right, Value& result, X87Op op) {
    storeInMemory(left);
    storeInMemory(right);
    storeInMemory(result);
    assembly << instructionSet->loadX87(memoryOperand(left), 16);
    assembly << instructionSet->loadX87(memoryOperand(right), 16);
    switch (op) {
    case X87Op::Add:
        assembly << instructionSet->faddp();
        break;
    case X87Op::Sub:
        assembly << instructionSet->fsubp();
        break;
    case X87Op::Mul:
        assembly << instructionSet->fmulp();
        break;
    case X87Op::Div:
        assembly << instructionSet->fdivp();
        break;
    }
    assembly << instructionSet->storeX87(memoryOperand(result), 16);
}

void StackMachine::emitX87UnaryMinus(Value& operand, Value& result) {
    storeInMemory(operand);
    storeInMemory(result);
    assembly << instructionSet->loadX87(memoryOperand(operand), 16);
    assembly << instructionSet->fchs();
    assembly << instructionSet->storeX87(memoryOperand(result), 16);
}

void StackMachine::emitX87Compare(Value& left, Value& right, bool signedRel) {
    Register& acc = get64BitRegister();
    storeInMemory(left);
    storeInMemory(right);
    assembly << instructionSet->loadX87(memoryOperand(right), 16);
    assembly << instructionSet->loadX87(memoryOperand(left), 16);
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    if (!signedRel) {
        return;
    }
    const int id = ++wideLabel_;
    const std::string done = "__xc" + std::to_string(id) + "d";
    assembly << instructionSet->mov("0", acc);
    assembly << instructionSet->je(done);
    assembly << instructionSet->mov("-1", acc);
    assembly << instructionSet->jb(done);
    assembly << instructionSet->mov("1", acc);
    assembly.label(instructionSet->label(done));
    setCompareFlagsFromTernary(acc, true);
}

void StackMachine::emitX87ZeroCompare(Value& symbol) {
    storeInMemory(symbol);
    assembly << instructionSet->loadX87(memoryOperand(symbol), 16);
    assembly << instructionSet->fldz();
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
}

void StackMachine::emitX87Convert(Value& operand, Value& result) {
    storeInMemory(operand);
    storeInMemory(result);
    const bool srcX = isX87Float(operand);
    const bool dstX = isX87Float(result);
    const bool srcF = operand.getValueKind() == ValueKind::FLOATING;
    const bool dstF = result.getValueKind() == ValueKind::FLOATING;
    if (!srcF && dstX) {
        assembly << instructionSet->fild(memoryOperand(operand), operand.getSizeInBytes() >= 8 ? 8 : 4);
        assembly << instructionSet->storeX87(memoryOperand(result), 16);
        return;
    }
    if (srcX && !dstF) {
        assembly << instructionSet->loadX87(memoryOperand(operand), 16);
        assembly << instructionSet->fisttp(memoryOperand(result), result.getSizeInBytes() >= 8 ? 8 : 4);
        return;
    }
    if (srcF && dstX) {
        assembly << instructionSet->loadX87(memoryOperand(operand), isSseFloat32(operand) ? 4 : 8);
        assembly << instructionSet->storeX87(memoryOperand(result), 16);
        return;
    }
    if (srcX && dstF) {
        assembly << instructionSet->loadX87(memoryOperand(operand), 16);
        assembly << instructionSet->storeX87(memoryOperand(result), isSseFloat32(result) ? 4 : 8);
    }
}

} // namespace codegen
