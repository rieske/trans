#ifndef CODEGEN_IR_BUILDERS_H_
#define CODEGEN_IR_BUILDERS_H_

#include "codegen/IrStringTable.h"
#include "codegen/JumpCondition.h"
#include "symbols/AddressPlan.h"

namespace codegen {
namespace ir {
namespace detail {

inline Instruction binary(Op op, int left, int right, int result) {
    Instruction i;
    i.op = op;
    i.arg0 = left;
    i.arg1 = right;
    i.result = result;
    return i;
}

inline Instruction unary(Op op, int operand, int result) {
    Instruction i;
    i.op = op;
    i.arg0 = operand;
    i.result = result;
    return i;
}

} // namespace detail

inline Instruction add(int left, int right, int result) {
    return detail::binary(Op::Add, left, right, result);
}
inline Instruction sub(int left, int right, int result) {
    return detail::binary(Op::Sub, left, right, result);
}
inline Instruction mul(int left, int right, int result) {
    return detail::binary(Op::Mul, left, right, result);
}
inline Instruction div(int left, int right, int result, bool signedDiv = true) {
    Instruction i = detail::binary(Op::Div, left, right, result);
    i.imm = signedDiv ? 1 : 0;
    return i;
}
inline Instruction mod(int left, int right, int result, bool signedDiv = true) {
    Instruction i = detail::binary(Op::Mod, left, right, result);
    i.imm = signedDiv ? 1 : 0;
    return i;
}
inline Instruction andOp(int left, int right, int result) {
    return detail::binary(Op::And, left, right, result);
}
inline Instruction orOp(int left, int right, int result) {
    return detail::binary(Op::Or, left, right, result);
}
inline Instruction xorOp(int left, int right, int result) {
    return detail::binary(Op::Xor, left, right, result);
}
inline Instruction shl(int left, int right, int result) {
    return detail::binary(Op::Shl, left, right, result);
}
inline Instruction shr(int left, int right, int result, bool arithmetic = true) {
    Instruction i = detail::binary(Op::Shr, left, right, result);
    i.imm = arithmetic ? 1 : 0;
    return i;
}
inline Instruction unaryMinus(int operand, int result) {
    return detail::unary(Op::UnaryMinus, operand, result);
}
inline Instruction unaryNot(int operand, int result) {
    return detail::unary(Op::UnaryNot, operand, result);
}
inline Instruction inc(int operand, int step = 1) {
    Instruction i;
    i.op = Op::Inc;
    i.arg0 = operand;
    i.imm = step;
    return i;
}
inline Instruction dec(int operand, int step = 1) {
    Instruction i;
    i.op = Op::Dec;
    i.arg0 = operand;
    i.imm = step;
    return i;
}
inline Instruction assign(int operand, int result) {
    return detail::unary(Op::Assign, operand, result);
}
inline Instruction widen(int operand, int result, bool signHighWord) {
    Instruction i = detail::unary(Op::Widen, operand, result);
    i.imm = signHighWord ? 1 : 0;
    return i;
}
inline Instruction assignConstant(int constant, int result) {
    Instruction i;
    i.op = Op::AssignConstant;
    i.arg0 = constant;
    i.result = result;
    return i;
}
inline Instruction assignConstant(int low, int high, int result) {
    Instruction i;
    i.op = Op::AssignConstant;
    i.arg0 = low;
    i.arg1 = high;
    i.result = result;
    return i;
}
inline Instruction assignLabelAddress(int labelName, int result) {
    Instruction i;
    i.op = Op::AssignLabelAddress;
    i.arg0 = labelName;
    i.result = result;
    return i;
}
inline Instruction lvalueAssign(int operand, int result) {
    return detail::unary(Op::LvalueAssign, operand, result);
}
inline Instruction addressOf(int operand, int result) {
    return detail::unary(Op::AddressOf, operand, result);
}
inline Instruction dereference(int operand, int lvalue, int result) {
    Instruction i;
    i.op = Op::Dereference;
    i.arg0 = operand;
    i.arg1 = lvalue;
    i.result = result;
    return i;
}
inline Instruction indexAddress(int base, int index, int elementSizeBytes, int result,
        symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject) {
    Instruction i;
    i.op = Op::IndexAddress;
    i.arg0 = base;
    i.arg1 = index;
    i.result = result;
    i.imm = elementSizeBytes;
    i.baseMode = baseMode;
    return i;
}
inline Instruction fieldAddress(int base, int offsetBytes, int result,
        symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject) {
    Instruction i;
    i.op = Op::FieldAddress;
    i.arg0 = base;
    i.result = result;
    i.imm = offsetBytes;
    i.baseMode = baseMode;
    return i;
}
inline Instruction copyPart(int source, int dest, int byteOffset) {
    Instruction i;
    i.op = Op::CopyPart;
    i.arg0 = source;
    i.result = dest;
    i.imm = byteOffset;
    return i;
}
inline Instruction pointerOffset(int base, int index, int elementSizeBytes, int result, bool subtract) {
    Instruction i;
    i.op = Op::PointerOffset;
    i.arg0 = base;
    i.arg1 = index;
    i.result = result;
    i.imm = elementSizeBytes;
    i.pointerSubtract = subtract;
    return i;
}
inline Instruction pointerDiff(int left, int right, int elementSizeBytes, int result) {
    Instruction i;
    i.op = Op::PointerDiff;
    i.arg0 = left;
    i.arg1 = right;
    i.result = result;
    i.imm = elementSizeBytes;
    return i;
}
inline Instruction functionAddress(int functionName, int result) {
    return detail::unary(Op::FunctionAddress, functionName, result);
}
inline Instruction valueCompare(int left, int right, bool signedRel = true) {
    Instruction i;
    i.op = Op::ValueCompare;
    i.arg0 = left;
    i.arg1 = right;
    i.imm = signedRel ? 1 : 0;
    return i;
}
inline Instruction zeroCompare(int symbol) {
    Instruction i;
    i.op = Op::ZeroCompare;
    i.arg0 = symbol;
    return i;
}
inline Instruction jump(int labelName, JumpCondition condition = JumpCondition::UNCONDITIONAL,
        bool signedRel = true) {
    Instruction i;
    i.op = Op::Jump;
    i.arg0 = labelName;
    i.cond = condition;
    i.imm = signedRel ? 1 : 0;
    return i;
}
inline Instruction label(int name) {
    Instruction i;
    i.op = Op::Label;
    i.arg0 = name;
    return i;
}
inline Instruction argument(int name) {
    Instruction i;
    i.op = Op::Argument;
    i.arg0 = name;
    return i;
}
inline Instruction call(int procedureName, bool indirect = false, int memoryReturnDest = kNoSymbol) {
    Instruction i;
    i.op = Op::Call;
    i.arg0 = procedureName;
    i.callIndirect = indirect;
    i.memoryReturnDest = memoryReturnDest;
    return i;
}
inline Instruction retrieve(int resultName, bool memoryReturn = false) {
    Instruction i;
    i.op = Op::Retrieve;
    i.result = resultName;
    i.memoryReturn = memoryReturn;
    return i;
}
inline Instruction ret(int returnSymbol) {
    Instruction i;
    i.op = Op::Return;
    i.arg0 = returnSymbol;
    return i;
}
inline Instruction voidReturn() {
    Instruction i;
    i.op = Op::VoidReturn;
    return i;
}
inline Instruction vaStart(int apPtr, int lastAddr = kNoSymbol) {
    Instruction i;
    i.op = Op::VaStart;
    i.arg0 = apPtr;
    i.arg1 = lastAddr;
    return i;
}
inline Instruction vaArg(int apPtr, int result) {
    Instruction i;
    i.op = Op::VaArg;
    i.arg0 = apPtr;
    i.result = result;
    return i;
}
inline Instruction vaCopy(int dstPtr, int srcPtr) {
    Instruction i;
    i.op = Op::VaCopy;
    i.arg0 = dstPtr;
    i.arg1 = srcPtr;
    return i;
}
inline Instruction vaEnd() {
    Instruction i;
    i.op = Op::VaEnd;
    return i;
}
inline Instruction bswap(int operand, int result, int widthBytes) {
    Instruction i;
    i.op = Op::Bswap;
    i.arg0 = operand;
    i.result = result;
    i.imm = widthBytes;
    return i;
}
inline Instruction ctz(int operand, int result, int widthBytes) {
    Instruction i;
    i.op = Op::Ctz;
    i.arg0 = operand;
    i.result = result;
    i.imm = widthBytes;
    return i;
}
inline Instruction allocaBytes(int size, int result) {
    Instruction i;
    i.op = Op::Alloca;
    i.arg0 = size;
    i.result = result;
    return i;
}

} // namespace ir
} // namespace codegen

#endif // CODEGEN_IR_BUILDERS_H_
