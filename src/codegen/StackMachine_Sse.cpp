#include "StackMachine.h"

#include <stdexcept>

#include "InstructionSet.h"

namespace codegen {

void StackMachine::gprToXmm(const Register& gpr, int xmmIndex, bool destFloat32) {
    if (destFloat32) {
        assembly << instructionSet->movdGprToXmm(gpr, xmmIndex);
    } else {
        assembly << instructionSet->movqGprToXmm(gpr, xmmIndex);
    }
}

void StackMachine::xmmToGpr(int xmmIndex, Register& gpr, bool destFloat32) {
    if (destFloat32) {
        assembly << instructionSet->movdXmmToGpr(xmmIndex, gpr);
    } else {
        assembly << instructionSet->movqXmmToGpr(xmmIndex, gpr);
    }
}

void StackMachine::loadValueToXmm(Value& v, int xmmIndex, bool destFloat32) {
    Register& tmp = get64BitRegister();
    if (residesInMemory(v)) {
        emitLoad(v, tmp);
    } else {
        assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
    }
    if (v.getType() == Type::INTEGRAL) {
        if (destFloat32) {
            assembly << instructionSet->cvtsi2ss(tmp, xmmIndex);
        } else {
            assembly << instructionSet->cvtsi2sd(tmp, xmmIndex);
        }
        return;
    }
    const bool srcFloat32 = isSseFloat32(v);
    gprToXmm(tmp, xmmIndex, srcFloat32);
    if (srcFloat32 && !destFloat32) {
        assembly << instructionSet->cvtss2sd(xmmIndex, xmmIndex);
    } else if (!srcFloat32 && destFloat32) {
        assembly << instructionSet->cvtsd2ss(xmmIndex, xmmIndex);
    }
}

void StackMachine::emitFloatingBinary(Value& left, Value& right, Value& result,
        std::string (InstructionSet::*ssOp)(int, int) const,
        std::string (InstructionSet::*sdOp)(int, int) const) {
    const bool destFloat32 = isSseFloat32(result);
    Register& resultRegister = get64BitRegister();
    loadValueToXmm(left, 0, destFloat32);
    loadValueToXmm(right, 1, destFloat32);
    auto op = destFloat32 ? ssOp : sdOp;
    assembly << (instructionSet.get()->*op)(0, 1);
    xmmToGpr(0, resultRegister, destFloat32);
    bindResult(resultRegister, result);
}

void StackMachine::emitFloatingOrX87Binary(Value& left, Value& right, Value& result,
        std::string (InstructionSet::*ssOp)(int, int) const,
        std::string (InstructionSet::*sdOp)(int, int) const,
        X87Op op) {
    if (tryX87Binary(left, right, result, op)) {
        return;
    }
    emitFloatingBinary(left, right, result, ssOp, sdOp);
}

bool StackMachine::tryNumericAssignConvert(Value& operand, Value& result) {
    if (operand.getType() == Type::COMPLEX || result.getType() == Type::COMPLEX) {
        return false;
    }
    const bool srcF = operand.getType() == Type::FLOATING;
    const bool dstF = result.getType() == Type::FLOATING;
    if (!srcF && !dstF) {
        return false;
    }
    if (isX87Float(operand) && isX87Float(result)) {
        return false;
    }
    if (isX87Float(operand) || isX87Float(result)) {
        emitX87Convert(operand, result);
        return true;
    }
    if (srcF && dstF && isSseFloat32(operand) == isSseFloat32(result)) {
        return false;
    }

    Register& src = residesInMemory(operand) ? assignRegisterTo(operand) : operand.getAssignedRegister();
    Register& dst = residesInMemory(result) ? get64BitRegisterExcluding(src) : result.getAssignedRegister();

    if (srcF && !dstF) {
        gprToXmm(src, 0, isSseFloat32(operand));
        if (isSseFloat32(operand)) {
            assembly << instructionSet->cvttss2si(0, dst);
        } else {
            assembly << instructionSet->cvttsd2si(0, dst);
        }
    } else if (!srcF && dstF) {
        if (isSseFloat32(result)) {
            assembly << instructionSet->cvtsi2ss(src, 0);
        } else {
            assembly << instructionSet->cvtsi2sd(src, 0);
        }
        xmmToGpr(0, dst, isSseFloat32(result));
    } else {
        gprToXmm(src, 0, isSseFloat32(operand));
        if (isSseFloat32(operand)) {
            assembly << instructionSet->cvtss2sd(0, 0);
        } else {
            assembly << instructionSet->cvtsd2ss(0, 0);
        }
        xmmToGpr(0, dst, isSseFloat32(result));
    }

    if (residesInMemory(result)) {
        emitStore(dst, result);
    } else {
        bindResult(dst, result);
    }
    return true;
}

} // namespace codegen
