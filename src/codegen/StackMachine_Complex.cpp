#include "StackMachine.h"

#include "InstructionSet.h"

namespace codegen {
namespace {

int complexPartBytes(const Value& v) {
    if (v.getSizeInBytes() == 8) {
        return 4;
    }
    if (v.getSizeInBytes() == 16) {
        return 8;
    }
    return 16;
}

int realX87Bytes(const Value& v) {
    if (v.getType() == Type::COMPLEX) {
        return complexPartBytes(v);
    }
    if (v.getType() == Type::FLOATING) {
        if (v.getSizeInBytes() == 4) {
            return 4;
        }
        if (v.getSizeInBytes() == 8) {
            return 8;
        }
        return 16;
    }
    return v.getSizeInBytes() >= 8 ? 8 : 4;
}

} // namespace

bool StackMachine::tryComplexBinary(Value& left, Value& right, Value& result, X87Op op) {
    if (result.getType() != Type::COMPLEX || (op != X87Op::Add && op != X87Op::Sub)) {
        return false;
    }
    emitComplexBinary(left, right, result, op);
    return true;
}

void StackMachine::emitComplexBinary(Value& left, Value& right, Value& result, X87Op op) {
    storeInMemory(left);
    storeInMemory(right);
    storeInMemory(result);
    const int part = complexPartBytes(result);
    for (int off = 0; off < result.getSizeInBytes(); off += part) {
        assembly << instructionSet->loadX87(memoryOperandAt(left, off), part);
        assembly << instructionSet->loadX87(memoryOperandAt(right, off), part);
        if (op == X87Op::Sub) {
            assembly << instructionSet->fsubp();
        } else {
            assembly << instructionSet->faddp();
        }
        assembly << instructionSet->storeX87(memoryOperandAt(result, off), part);
    }
}

void StackMachine::emitComplexUnaryMinus(Value& operand, Value& result) {
    storeInMemory(operand);
    storeInMemory(result);
    const int part = complexPartBytes(operand);
    for (int off = 0; off < operand.getSizeInBytes(); off += part) {
        assembly << instructionSet->loadX87(memoryOperandAt(operand, off), part);
        assembly << instructionSet->fchs();
        assembly << instructionSet->storeX87(memoryOperandAt(result, off), part);
    }
}

void StackMachine::emitComplexCompare(Value& left, Value& right) {
    storeInMemory(left);
    storeInMemory(right);
    const int part = complexPartBytes(left);
    const int id = ++wideLabel_;
    const std::string done = "__ccmp" + std::to_string(id) + "d";
    assembly << instructionSet->loadX87(memoryOperandAt(left, 0), part);
    assembly << instructionSet->loadX87(memoryOperandAt(right, 0), part);
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly << instructionSet->jne(done);
    assembly << instructionSet->loadX87(memoryOperandAt(left, part), part);
    assembly << instructionSet->loadX87(memoryOperandAt(right, part), part);
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly.label(instructionSet->label(done));
}

void StackMachine::emitComplexZeroCompare(Value& symbol) {
    storeInMemory(symbol);
    const int part = complexPartBytes(symbol);
    const int id = ++wideLabel_;
    const std::string done = "__cz" + std::to_string(id) + "d";
    assembly << instructionSet->loadX87(memoryOperandAt(symbol, 0), part);
    assembly << instructionSet->fldz();
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly << instructionSet->jne(done);
    assembly << instructionSet->loadX87(memoryOperandAt(symbol, part), part);
    assembly << instructionSet->fldz();
    assembly << instructionSet->fucomip();
    assembly << instructionSet->fstpSt0();
    assembly.label(instructionSet->label(done));
}

void StackMachine::loadValuePartToX87(Value& v, int byteOffset) {
    if (v.getType() == Type::INTEGRAL) {
        assembly << instructionSet->fild(memoryOperandAt(v, byteOffset), realX87Bytes(v));
        return;
    }
    assembly << instructionSet->loadX87(memoryOperandAt(v, byteOffset), realX87Bytes(v));
}

void StackMachine::storeX87ToValuePart(Value& v, int byteOffset) {
    if (v.getType() == Type::INTEGRAL) {
        assembly << instructionSet->fisttp(memoryOperandAt(v, byteOffset), realX87Bytes(v));
        return;
    }
    assembly << instructionSet->storeX87(memoryOperandAt(v, byteOffset), realX87Bytes(v));
}

void StackMachine::zeroComplexImag(Value& dest) {
    const int part = complexPartBytes(dest);
    assembly << instructionSet->fldz();
    assembly << instructionSet->storeX87(memoryOperandAt(dest, part), part);
}

void StackMachine::emitComplexLibgccCall(Value& left, Value& right, Value& result, bool divide) {
    storeInMemory(left);
    storeInMemory(right);
    storeInMemory(result);
    const int part = complexPartBytes(result);
    const char* helper = divide
            ? (part == 4 ? "__divsc3" : part == 8 ? "__divdc3" : "__divxc3")
            : (part == 4 ? "__mulsc3" : part == 8 ? "__muldc3" : "__mulxc3");

    spillCallerSavedRegisters();
    Register& rax = registers->getMultiplicationRegister();
    storeRegisterValue(rax);

    Value* srcs[2] = { &left, &right };
    if (part == 16) {
        for (int i = 0; i < 4; ++i) {
            assembly << instructionSet->loadX87(
                    memoryOperandAt(*srcs[i / 2], (i % 2) * part), part);
        }
        for (int i = 0; i < 4; ++i) {
            assembly << instructionSet->sub(registers->getStackPointer(), 16);
            assembly << instructionSet->storeX87(
                    MemoryOperand::at(registers->getStackPointer(), 0), part);
        }
        assembly << instructionSet->xor_(rax, rax);
    } else {
        Register& tmp = get64BitRegisterExcluding(rax);
        for (int i = 0; i < 4; ++i) {
            MemoryOperand mem = memoryOperandAt(*srcs[i / 2], (i % 2) * part);
            if (part == 4) {
                assembly << instructionSet->movDword(mem, tmp);
            } else {
                assembly << instructionSet->mov(mem, tmp);
            }
            gprToXmm(tmp, i, part == 4);
        }
        assembly << instructionSet->mov("4", rax);
    }

    assembly.raw(instructionSet->externDirective(helper) + "\n");
    if (isDefinedProcedure(helper)) {
        assembly << instructionSet->call(helper);
    } else {
        assembly << instructionSet->callPlt(helper);
    }

    if (part == 16) {
        assembly << instructionSet->add(registers->getStackPointer(), 64);
        emitComplexX87Store(result);
    } else {
        retrieveProcedureReturnValue(result.getName());
    }
}

bool StackMachine::tryComplexAssignConvert(Value& operand, Value& result) {
    const bool srcC = operand.getType() == Type::COMPLEX;
    const bool dstC = result.getType() == Type::COMPLEX;
    if (!srcC && !dstC) {
        return false;
    }
    if (srcC && dstC && operand.getSizeInBytes() == result.getSizeInBytes()) {
        return false;
    }

    storeInMemory(operand);
    storeInMemory(result);
    if (operand.getType() == Type::INTEGRAL && operand.getSizeInBytes() < 4) {
        Register& src = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
        assembly << instructionSet->movDword(src, memoryOperandAt(result, 0));
        assembly << instructionSet->fild(memoryOperandAt(result, 0), 4);
        storeX87ToValuePart(result, 0);
        if (dstC) {
            zeroComplexImag(result);
        }
        return true;
    }

    loadValuePartToX87(operand, 0);
    storeX87ToValuePart(result, 0);
    if (dstC) {
        if (srcC) {
            loadValuePartToX87(operand, realX87Bytes(operand));
            storeX87ToValuePart(result, realX87Bytes(result));
        } else {
            zeroComplexImag(result);
        }
    }
    return true;
}

} // namespace codegen
