#include "StackMachine.h"

#include "types/ObjectAbi.h"

#include "InstructionSet.h"

namespace codegen {

bool StackMachine::isMultiWord(const Value& v) const {
    return type::object_abi::valueWords(v.getSizeInBytes()) > 1;
}

bool StackMachine::tryWideIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp op) {
    if (!isMultiWord(left) && !isMultiWord(right) && !isMultiWord(result)) {
        return false;
    }
    wideIntegerBinary(left, right, result, op);
    return true;
}

bool StackMachine::tryWideUnaryMinus(Value& operand, Value& result) {
    if (!isMultiWord(operand) && !isMultiWord(result)) {
        return false;
    }
    wideUnaryMinus(operand, result);
    return true;
}

bool StackMachine::tryWideUnaryNot(Value& operand, Value& result) {
    if (!isMultiWord(operand) && !isMultiWord(result)) {
        return false;
    }
    wideUnaryNot(operand, result);
    return true;
}

bool StackMachine::tryWideCompare(Value& left, Value& right, bool signedRel) {
    if (!isMultiWord(left) && !isMultiWord(right)) {
        return false;
    }
    wideCompare(left, right, signedRel);
    return true;
}

bool StackMachine::tryWideZeroCompare(Value& symbol) {
    if (!isMultiWord(symbol)) {
        return false;
    }
    wideZeroCompare(symbol);
    return true;
}

void StackMachine::wideIntegerBinary(Value& left, Value& right, Value& result, WideIntegerOp op) {
    storeInMemory(left);
    storeInMemory(right);
    Register& acc = get64BitRegister();
    Register& tmp = get64BitRegisterExcluding(acc);
    const int words = type::object_abi::valueWords(result.getSizeInBytes());
    for (int w = 0; w < words; ++w) {
        loadWord(left, w, acc);
        loadWord(right, w, tmp);
        const bool carry = w > 0;
        switch (op) {
        case WideIntegerOp::Add:
            assembly << (carry ? instructionSet->adc(tmp, acc) : instructionSet->add(tmp, acc));
            break;
        case WideIntegerOp::Sub:
            assembly << (carry ? instructionSet->sbb(tmp, acc) : instructionSet->sub(tmp, acc));
            break;
        case WideIntegerOp::And:
            assembly << instructionSet->and_(tmp, acc);
            break;
        case WideIntegerOp::Or:
            assembly << instructionSet->or_(tmp, acc);
            break;
        case WideIntegerOp::Xor:
            assembly << instructionSet->xor_(tmp, acc);
            break;
        }
        storeWord(acc, result, w);
    }
}

void StackMachine::wideUnaryMinus(Value& operand, Value& result) {
    storeInMemory(operand);
    Register& lo = get64BitRegister();
    Register& hi = get64BitRegisterExcluding(lo);
    Register& zero = get64BitRegisterExcluding(std::vector<Register*> { &lo, &hi });
    loadWord(operand, 0, lo);
    loadWord(operand, 1, hi);
    assembly << instructionSet->xor_(zero, zero);
    assembly << instructionSet->neg(lo);
    assembly << instructionSet->adc(zero, hi);
    assembly << instructionSet->neg(hi);
    storeWord(lo, result, 0);
    storeWord(hi, result, 1);
}

void StackMachine::wideUnaryNot(Value& operand, Value& result) {
    storeInMemory(operand);
    Register& acc = get64BitRegister();
    const int words = type::object_abi::valueWords(result.getSizeInBytes());
    for (int w = 0; w < words; ++w) {
        loadWord(operand, w, acc);
        assembly << instructionSet->not_(acc);
        storeWord(acc, result, w);
    }
}

void StackMachine::wideCompare(Value& left, Value& right, bool signedRel) {
    storeInMemory(left);
    storeInMemory(right);
    Register& l = get64BitRegister();
    Register& r = get64BitRegisterExcluding(l);
    Register& acc = get64BitRegisterExcluding(std::vector<Register*> { &l, &r });
    assembly << instructionSet->xor_(acc, acc);
    const int id = ++wideLabel_;
    const std::string lows = "__wc" + std::to_string(id) + "l";
    const std::string done = "__wc" + std::to_string(id) + "d";
    loadWord(left, 1, l);
    loadWord(right, 1, r);
    assembly << instructionSet->cmp(l, r);
    assembly << instructionSet->je(lows);
    assembly << instructionSet->mov("-1", acc);
    assembly << (signedRel ? instructionSet->jl(done) : instructionSet->jb(done));
    assembly << instructionSet->mov("1", acc);
    assembly << instructionSet->jmp(done);
    assembly.label(instructionSet->label(lows));
    loadWord(left, 0, l);
    loadWord(right, 0, r);
    assembly << instructionSet->cmp(l, r);
    assembly << instructionSet->je(done);
    assembly << instructionSet->mov("-1", acc);
    assembly << instructionSet->jb(done);
    assembly << instructionSet->mov("1", acc);
    assembly.label(instructionSet->label(done));
    if (signedRel) {
        assembly << instructionSet->cmp(acc, 0);
    } else {
        // acc in {-1,0,1} -> {0,1,2} so ja/jb match the following unsigned jump.
        assembly << instructionSet->add(acc, 1);
        assembly << instructionSet->cmp(acc, 1);
    }
}

void StackMachine::wideZeroCompare(Value& symbol) {
    storeInMemory(symbol);
    Register& acc = get64BitRegister();
    Register& tmp = get64BitRegisterExcluding(acc);
    loadWord(symbol, 0, acc);
    loadWord(symbol, 1, tmp);
    assembly << instructionSet->or_(tmp, acc);
    assembly << instructionSet->cmp(acc, 0);
}

bool StackMachine::tryWideShift(Value& value, Value& count, Value& result, WideShiftOp op) {
    if (!isMultiWord(value) && !isMultiWord(result)) {
        return false;
    }
    wideShift(value, count, result, op);
    return true;
}

void StackMachine::wideShift(Value& value, Value& count, Value& result, WideShiftOp op) {
    storeInMemory(value);
    storeInMemory(count);
    Register& rcx = getCounterRegister();
    loadWord(count, 0, rcx);
    Register& lo = get64BitRegisterExcluding(rcx);
    Register& hi = get64BitRegisterExcluding(std::vector<Register*> { &rcx, &lo });
    loadWord(value, 0, lo);
    loadWord(value, 1, hi);
    const int id = ++wideLabel_;
    const std::string ge64 = "__ws" + std::to_string(id) + "g";
    const std::string done = "__ws" + std::to_string(id) + "d";
    assembly << instructionSet->cmp(rcx, 63);
    assembly << instructionSet->ja(ge64);
    switch (op) {
    case WideShiftOp::Left:
        assembly << instructionSet->shld(lo, hi);
        assembly << instructionSet->shl(lo);
        break;
    case WideShiftOp::ArithmeticRight:
        assembly << instructionSet->shrd(hi, lo);
        assembly << instructionSet->shr(hi);
        break;
    case WideShiftOp::LogicalRight:
        assembly << instructionSet->shrd(hi, lo);
        assembly << instructionSet->lshr(hi);
        break;
    }
    assembly << instructionSet->jmp(done);
    assembly.label(instructionSet->label(ge64));
    assembly << instructionSet->sub(rcx, 64);
    switch (op) {
    case WideShiftOp::Left:
        assembly << instructionSet->mov(lo, hi);
        assembly << instructionSet->xor_(lo, lo);
        assembly << instructionSet->shl(hi);
        break;
    case WideShiftOp::ArithmeticRight:
        assembly << instructionSet->mov(hi, lo);
        assembly << instructionSet->shr(lo);
        assembly << instructionSet->mov("63", rcx);
        assembly << instructionSet->shr(hi);
        break;
    case WideShiftOp::LogicalRight:
        assembly << instructionSet->mov(hi, lo);
        assembly << instructionSet->lshr(lo);
        assembly << instructionSet->xor_(hi, hi);
        break;
    }
    assembly.label(instructionSet->label(done));
    storeWord(lo, result, 0);
    storeWord(hi, result, 1);
}

} // namespace codegen
