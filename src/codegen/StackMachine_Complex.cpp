#include "StackMachine.h"

#include "InstructionSet.h"
#include "MemoryOperand.h"
#include "codegen/InternalError.h"
#include "types/SysVClass.h"

namespace codegen {
namespace {

int complexPartBytes(const Value& v) {
    const type::sysv::Classification cls = v.getClassification();
    if (type::sysv::isComplexX87(cls)) {
        return 16;
    }
    if (cls.count == 2 && type::sysv::isSse(cls.eightbytes[0])) {
        return 8;
    }
    if (cls.count == 1 && type::sysv::isSse(cls.eightbytes[0])) {
        return 4;
    }
    internalError("complexPartBytes: value is not a classified complex");
}

int realX87Bytes(const Value& v) {
    return v.getType() == Type::COMPLEX ? complexPartBytes(v) : v.getSizeInBytes();
}

} // namespace

MemoryOperand StackMachine::partOperand(Value& symbol, int byteOffset) {
    storeInMemory(symbol);
    Address home = addressOf(symbol);
    if (home.isGlobal()) {
        Register& addr = get64BitRegister();
        assembly << instructionSet->lea(memoryOperand(home), addr);
        return MemoryOperand::at(addr, byteOffset);
    }
    return memoryOperandAt(symbol, byteOffset);
}

void StackMachine::loadX87At(Value& symbol, int byteOffset, int sizeBytes) {
    assembly << instructionSet->loadX87(partOperand(symbol, byteOffset), sizeBytes);
}

void StackMachine::storeX87At(Value& symbol, int byteOffset, int sizeBytes) {
    assembly << instructionSet->storeX87(partOperand(symbol, byteOffset), sizeBytes);
}

bool StackMachine::tryComplexBinary(Value& left, Value& right, Value& result, X87Op op) {
    if (result.getType() != Type::COMPLEX || (op != X87Op::Add && op != X87Op::Sub)) {
        return false;
    }
    emitComplexBinary(left, right, result, op);
    return true;
}

bool StackMachine::tryComplexUnaryMinus(Value& operand, Value& result) {
    if (operand.getType() != Type::COMPLEX) {
        return false;
    }
    emitComplexUnaryMinus(operand, result);
    return true;
}

bool StackMachine::tryComplexCompare(Value& left, Value& right) {
    if (left.getType() != Type::COMPLEX && right.getType() != Type::COMPLEX) {
        return false;
    }
    emitComplexCompare(left, right);
    return true;
}

bool StackMachine::tryComplexZeroCompare(Value& symbol) {
    if (symbol.getType() != Type::COMPLEX) {
        return false;
    }
    emitComplexZeroCompare(symbol);
    return true;
}

void StackMachine::emitComplexBinary(Value& left, Value& right, Value& result, X87Op op) {
    const int part = complexPartBytes(result);
    for (int off = 0; off < result.getSizeInBytes(); off += part) {
        loadX87At(left, off, part);
        loadX87At(right, off, part);
        if (op == X87Op::Sub) {
            assembly << instructionSet->fsubp();
        } else {
            assembly << instructionSet->faddp();
        }
        storeX87At(result, off, part);
    }
}

void StackMachine::emitComplexUnaryMinus(Value& operand, Value& result) {
    const int part = complexPartBytes(operand);
    for (int off = 0; off < operand.getSizeInBytes(); off += part) {
        loadX87At(operand, off, part);
        assembly << instructionSet->fchs();
        storeX87At(result, off, part);
    }
}

void StackMachine::emitComplexCompare(Value& left, Value& right) {
    const int part = complexPartBytes(left);
    const int id = ++wideLabel_;
    const std::string done = "__ccmp" + std::to_string(id) + "d";
    loadX87At(left, 0, part);
    loadX87At(right, 0, part);
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly << instructionSet->jne(done);
    loadX87At(left, part, part);
    loadX87At(right, part, part);
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly.label(instructionSet->label(done));
}

void StackMachine::emitComplexZeroCompare(Value& symbol) {
    const int part = complexPartBytes(symbol);
    const int id = ++wideLabel_;
    const std::string done = "__cz" + std::to_string(id) + "d";
    loadX87At(symbol, 0, part);
    assembly << instructionSet->fldz();
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly << instructionSet->jne(done);
    loadX87At(symbol, part, part);
    assembly << instructionSet->fldz();
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly.label(instructionSet->label(done));
}

void StackMachine::copyPart(int sourceName, int destName, int byteOffset) {
    Value& source = resolve(sourceName);
    Value& dest = resolve(destName);
    const int n = dest.getSizeInBytes();
    loadX87At(source, byteOffset, n);
    storeX87At(dest, 0, n);
}

void StackMachine::emitComplexX87Load(Value& symbol) {
    loadX87At(symbol, 16, 16);
    loadX87At(symbol, 0, 16);
}

void StackMachine::emitComplexX87Store(Value& symbol) {
    storeX87At(symbol, 0, 16);
    storeX87At(symbol, 16, 16);
}

bool StackMachine::tryComplexAssignConvert(Value& operand, Value& result, bool unsignedSource) {
    const bool srcC = operand.getType() == Type::COMPLEX;
    const bool dstC = result.getType() == Type::COMPLEX;
    if (!srcC && !dstC) {
        return false;
    }
    if (srcC && dstC && operand.getSizeInBytes() == result.getSizeInBytes()) {
        return false;
    }

    if (result.getType() == Type::INTEGRAL) {
        storeInMemory(operand);
        fisttpInteger(memoryOperand(operand), realX87Bytes(operand), result, unsignedSource);
        return true;
    }
    if (operand.getType() == Type::INTEGRAL) {
        fildInteger(operand, unsignedSource);
    } else {
        loadX87At(operand, 0, realX87Bytes(operand));
    }
    const int part = realX87Bytes(result);
    storeX87At(result, 0, part);
    if (dstC) {
        if (srcC) {
            loadX87At(operand, realX87Bytes(operand), realX87Bytes(operand));
        } else {
            assembly << instructionSet->fldz();
        }
        storeX87At(result, part, part);
    }
    return true;
}

} // namespace codegen
