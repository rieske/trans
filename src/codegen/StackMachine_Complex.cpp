#include "StackMachine.h"

#include "InstructionSet.h"
#include "MemoryOperand.h"

#include <stdexcept>

namespace codegen {
namespace {

int complexPartBytes(const Value& v) {
    if (v.getValueKind() != ValueKind::COMPLEX) {
        throw std::runtime_error { "complexPartBytes: value is not complex" };
    }
    switch (v.getSizeInBytes()) {
    case 8:
        return 4;
    case 16:
        return 8;
    case 32:
        return 16;
    default:
        throw std::runtime_error { "complexPartBytes: not an ISO complex size" };
    }
}

int realX87Bytes(const Value& v) {
    if (v.getValueKind() == ValueKind::COMPLEX) {
        return complexPartBytes(v);
    }
    if (v.getValueKind() == ValueKind::FLOATING) {
        if (isX87Float(v)) {
            return 16;
        }
        if (isSseFloat64(v)) {
            return 8;
        }
        if (isSseFloat32(v)) {
            return 4;
        }
        throw std::runtime_error { "realX87Bytes: unknown floating width" };
    }
    if (v.getValueKind() != ValueKind::INTEGRAL) {
        throw std::runtime_error { "realX87Bytes: not a numeric value" };
    }
    return v.getSizeInBytes() >= 8 ? 8 : 4;
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
    if (result.getValueKind() != ValueKind::COMPLEX || (op != X87Op::Add && op != X87Op::Sub)) {
        return false;
    }
    emitComplexBinary(left, right, result, op);
    return true;
}

bool StackMachine::tryComplexUnaryMinus(Value& operand, Value& result) {
    if (operand.getValueKind() != ValueKind::COMPLEX) {
        return false;
    }
    emitComplexUnaryMinus(operand, result);
    return true;
}

bool StackMachine::tryComplexCompare(Value& left, Value& right) {
    if (left.getValueKind() != ValueKind::COMPLEX && right.getValueKind() != ValueKind::COMPLEX) {
        return false;
    }
    emitComplexCompare(left, right);
    return true;
}

bool StackMachine::tryComplexZeroCompare(Value& symbol) {
    if (symbol.getValueKind() != ValueKind::COMPLEX) {
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

void StackMachine::copyPart(std::string sourceName, std::string destName, int byteOffset) {
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

bool StackMachine::tryComplexAssignConvert(Value& operand, Value& result) {
    const bool srcC = operand.getValueKind() == ValueKind::COMPLEX;
    const bool dstC = result.getValueKind() == ValueKind::COMPLEX;
    if (!srcC && !dstC) {
        return false;
    }
    if (srcC && dstC && operand.getSizeInBytes() == result.getSizeInBytes()) {
        return false;
    }

    auto loadPart = [&](Value& v, int off) {
        const MemoryOperand mem = partOperand(v, off);
        if (v.getValueKind() == ValueKind::INTEGRAL) {
            assembly << instructionSet->fild(mem, realX87Bytes(v));
        } else {
            assembly << instructionSet->loadX87(mem, realX87Bytes(v));
        }
    };
    auto storePart = [&](Value& v, int off) {
        const MemoryOperand mem = partOperand(v, off);
        if (v.getValueKind() == ValueKind::INTEGRAL) {
            assembly << instructionSet->fisttp(mem, realX87Bytes(v));
        } else {
            assembly << instructionSet->storeX87(mem, realX87Bytes(v));
        }
    };

    if (operand.getValueKind() == ValueKind::INTEGRAL) {
        Register& src = residesInMemory(operand)
                ? assignRegisterTo(operand) : operand.getAssignedRegister();
        assembly << instructionSet->push(src);
        assembly << instructionSet->fild(
                MemoryOperand::at(registers->getStackPointer(), 0),
                operand.getSizeInBytes() >= 8 ? 8 : 4);
        assembly << instructionSet->add(registers->getStackPointer(), 8);
        storePart(result, 0);
        if (dstC) {
            assembly << instructionSet->fldz();
            storePart(result, realX87Bytes(result));
        }
        return true;
    }

    loadPart(operand, 0);
    storePart(result, 0);
    if (dstC) {
        if (srcC) {
            loadPart(operand, realX87Bytes(operand));
            storePart(result, realX87Bytes(result));
        } else {
            assembly << instructionSet->fldz();
            storePart(result, realX87Bytes(result));
        }
    }
    return true;
}

} // namespace codegen
