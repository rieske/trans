#include "StackMachine.h"

#include "InstructionSet.h"
#include "util/ImmediateFormat.h"

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
    assembly << (instructionSet->*op)(0, 1);
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

bool StackMachine::tryNumericAssignConvert(Value& operand, Value& result, bool unsignedSource) {
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
    if (!srcF && dstF && unsignedSource && operand.getSizeInBytes() > 8) {
        emitUnsigned128ToFloat(operand, result);
        return true;
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
        const bool destFloat32 = isSseFloat32(result);
        // The shift doubles the magnitude, so it runs only when the high bit is set.
        // Adding 2^64 after cvtsi2sd would round a second time.
        if (unsignedSource && operand.getSizeInBytes() == 8) {
            const int id = ++wideLabel_;
            const std::string small = "__us" + std::to_string(id);
            const std::string done = "__uf" + std::to_string(id);
            Register& shifted = get64BitRegisterExcluding(src);
            Register& lowBit = get64BitRegisterExcluding(std::vector<Register*>{&src, &shifted});
            assembly << instructionSet->cmp(src, 0);
            assembly << instructionSet->jns(small);
            assembly << instructionSet->mov(src, shifted);
            assembly << instructionSet->lshrImm(shifted, 1);
            assembly << instructionSet->mov(src, lowBit);
            assembly << instructionSet->andImm(lowBit, 1);
            assembly << instructionSet->or_(lowBit, shifted);
            if (destFloat32) {
                assembly << instructionSet->cvtsi2ss(shifted, 0);
                assembly << instructionSet->addss(0, 0);
            } else {
                assembly << instructionSet->cvtsi2sd(shifted, 0);
                assembly << instructionSet->addsd(0, 0);
            }
            assembly << instructionSet->jmp(done);
            assembly.label(instructionSet->label(small));
            if (destFloat32) {
                assembly << instructionSet->cvtsi2ss(src, 0);
            } else {
                assembly << instructionSet->cvtsi2sd(src, 0);
            }
            assembly.label(instructionSet->label(done));
        } else if (destFloat32) {
            assembly << instructionSet->cvtsi2ss(src, 0);
        } else {
            assembly << instructionSet->cvtsi2sd(src, 0);
        }
        xmmToGpr(0, dst, destFloat32);
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

void StackMachine::emitUnsigned64ToXmm(Register& src, bool destFloat32,
        const std::vector<Register*>& keep) {
    std::vector<Register*> exclude = keep;
    exclude.push_back(&src);
    Register& shifted = get64BitRegisterExcluding(exclude);
    exclude.push_back(&shifted);
    Register& lowBit = get64BitRegisterExcluding(exclude);
    const int id = ++wideLabel_;
    const std::string small = "__us" + std::to_string(id);
    const std::string done = "__uf" + std::to_string(id);
    assembly << instructionSet->cmp(src, 0);
    assembly << instructionSet->jns(small);
    assembly << instructionSet->mov(src, shifted);
    assembly << instructionSet->lshrImm(shifted, 1);
    assembly << instructionSet->mov(src, lowBit);
    assembly << instructionSet->andImm(lowBit, 1);
    assembly << instructionSet->or_(lowBit, shifted);
    if (destFloat32) {
        assembly << instructionSet->cvtsi2ss(shifted, 0);
        assembly << instructionSet->addss(0, 0);
    } else {
        assembly << instructionSet->cvtsi2sd(shifted, 0);
        assembly << instructionSet->addsd(0, 0);
    }
    assembly << instructionSet->jmp(done);
    assembly.label(instructionSet->label(small));
    if (destFloat32) {
        assembly << instructionSet->cvtsi2ss(src, 0);
    } else {
        assembly << instructionSet->cvtsi2sd(src, 0);
    }
    assembly.label(instructionSet->label(done));
}

void StackMachine::emitUnsigned128ToFloat(Value& operand, Value& result) {
    const bool destFloat32 = isSseFloat32(result);
    storeInMemory(operand);
    Register& lo = get64BitRegister();
    Register& hi = get64BitRegisterExcluding(lo);
    loadWord(operand, 0, lo);
    loadWord(operand, 1, hi, 0, {&lo});
    const int id = ++wideLabel_;
    const std::string high = "__uh" + std::to_string(id);
    const std::string done = "__ud" + std::to_string(id);
    assembly << instructionSet->cmp(hi, 0);
    assembly << instructionSet->jne(high);
    emitUnsigned64ToXmm(lo, destFloat32, {&hi});
    assembly << instructionSet->jmp(done);
    assembly.label(instructionSet->label(high));
    emitUnsigned64ToXmm(hi, destFloat32, {&lo});
    Register& scale = get64BitRegisterExcluding(std::vector<Register*>{&lo, &hi});
    const unsigned long long scaleBits = destFloat32 ? 0x5f800000ull : 0x43f0000000000000ull;
    assembly << instructionSet->mov(util::hexImmediate(scaleBits), scale);
    gprToXmm(scale, 1, destFloat32);
    if (destFloat32) {
        assembly << instructionSet->mulss(0, 1);
    } else {
        assembly << instructionSet->mulsd(0, 1);
    }
    Register& saved = get64BitRegisterExcluding(std::vector<Register*>{&lo, &hi, &scale});
    xmmToGpr(0, saved, destFloat32);
    emitUnsigned64ToXmm(lo, destFloat32, {&saved});
    gprToXmm(saved, 1, destFloat32);
    if (destFloat32) {
        assembly << instructionSet->addss(0, 1);
    } else {
        assembly << instructionSet->addsd(0, 1);
    }
    assembly.label(instructionSet->label(done));
    Register& dst = residesInMemory(result) ? get64BitRegister() : result.getAssignedRegister();
    xmmToGpr(0, dst, destFloat32);
    if (residesInMemory(result)) {
        emitStore(dst, result);
    } else {
        bindResult(dst, result);
    }
}

} // namespace codegen
