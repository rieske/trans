#include "StackMachine.h"

#include "InstructionSet.h"

namespace codegen {

void StackMachine::gprToXmm(const Register& gpr, int xmmIndex, SseWidth width) {
    assembly << instructionSet->sseGprXmm(SseGprXmmDir::GprToXmm, width, gpr, xmmIndex);
}

void StackMachine::xmmToGpr(int xmmIndex, Register& gpr, SseWidth width) {
    assembly << instructionSet->sseGprXmm(SseGprXmmDir::XmmToGpr, width, gpr, xmmIndex);
}

void StackMachine::loadValueToXmm(Value& v, int xmmIndex, SseWidth destWidth) {
    Register& tmp = get64BitRegister();
    if (residesInMemory(v)) {
        emitLoad(v, tmp);
    } else {
        assembly << instructionSet->mov(v.getAssignedRegister(), tmp);
    }
    if (v.getValueKind() == ValueKind::INTEGRAL) {
        assembly << instructionSet->sseCvtIntToXmm(tmp, xmmIndex, destWidth);
        return;
    }
    const SseWidth srcWidth = sseWidth(v);
    gprToXmm(tmp, xmmIndex, srcWidth);
    if (srcWidth != destWidth) {
        assembly << instructionSet->sseCvtFloat(srcWidth, destWidth, xmmIndex, xmmIndex);
    }
}

void StackMachine::emitFloatingBinary(Value& left, Value& right, Value& result, SseBin op) {
    const SseWidth destWidth = sseWidth(result);
    Register& resultRegister = get64BitRegister();
    loadValueToXmm(left, 0, destWidth);
    loadValueToXmm(right, 1, destWidth);
    assembly << instructionSet->sseBin(op, destWidth, 0, 1);
    xmmToGpr(0, resultRegister, destWidth);
    bindResult(resultRegister, result);
}

bool StackMachine::tryNumericAssignConvert(Value& operand, Value& result) {
    const bool srcF = operand.getValueKind() == ValueKind::FLOATING;
    const bool dstF = result.getValueKind() == ValueKind::FLOATING;
    if (!srcF && !dstF) {
        return false;
    }
    if (srcF && dstF && sseWidth(operand) == sseWidth(result)) {
        return false;
    }

    const SseWidth destXmm = dstF ? sseWidth(result) : sseWidth(operand);
    loadValueToXmm(operand, 0, destXmm);

    Register& dst = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    if (dstF) {
        xmmToGpr(0, dst, sseWidth(result));
    } else {
        assembly << instructionSet->sseCvtTruncToGpr(0, dst, sseWidth(operand));
    }
    if (residesInMemory(result)) {
        emitStore(dst, result);
    } else {
        bindResult(dst, result);
    }
    return true;
}

} // namespace codegen
