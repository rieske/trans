#ifndef CODEGEN_IR_BUILDERS_H_
#define CODEGEN_IR_BUILDERS_H_

#include <string>
#include <utility>
#include <vector>

#include "codegen/JumpCondition.h"
#include "symbols/AddressPlan.h"

namespace codegen {
namespace ir {
namespace detail {

inline Instruction binary(Op op, std::string left, std::string right, std::string result) {
    Instruction i;
    i.op = op;
    i.arg0 = std::move(left);
    i.arg1 = std::move(right);
    i.result = std::move(result);
    return i;
}

inline Instruction unary(Op op, std::string operand, std::string result) {
    Instruction i;
    i.op = op;
    i.arg0 = std::move(operand);
    i.result = std::move(result);
    return i;
}

} // namespace detail

inline Instruction add(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Add, std::move(left), std::move(right), std::move(result));
}
inline Instruction sub(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Sub, std::move(left), std::move(right), std::move(result));
}
inline Instruction mul(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Mul, std::move(left), std::move(right), std::move(result));
}
inline Instruction div(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Div, std::move(left), std::move(right), std::move(result));
}
inline Instruction mod(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Mod, std::move(left), std::move(right), std::move(result));
}
inline Instruction andOp(std::string left, std::string right, std::string result) {
    return detail::binary(Op::And, std::move(left), std::move(right), std::move(result));
}
inline Instruction orOp(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Or, std::move(left), std::move(right), std::move(result));
}
inline Instruction xorOp(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Xor, std::move(left), std::move(right), std::move(result));
}
inline Instruction shl(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Shl, std::move(left), std::move(right), std::move(result));
}
inline Instruction shr(std::string left, std::string right, std::string result) {
    return detail::binary(Op::Shr, std::move(left), std::move(right), std::move(result));
}
inline Instruction unaryMinus(std::string operand, std::string result) {
    return detail::unary(Op::UnaryMinus, std::move(operand), std::move(result));
}
inline Instruction unaryNot(std::string operand, std::string result) {
    return detail::unary(Op::UnaryNot, std::move(operand), std::move(result));
}
inline Instruction inc(std::string operand, int step = 1) {
    Instruction i;
    i.op = Op::Inc;
    i.arg0 = std::move(operand);
    i.imm = step;
    return i;
}
inline Instruction dec(std::string operand, int step = 1) {
    Instruction i;
    i.op = Op::Dec;
    i.arg0 = std::move(operand);
    i.imm = step;
    return i;
}
inline Instruction assign(std::string operand, std::string result) {
    return detail::unary(Op::Assign, std::move(operand), std::move(result));
}
inline Instruction assignConstant(std::string constant, std::string result) {
    Instruction i;
    i.op = Op::AssignConstant;
    i.arg0 = std::move(constant);
    i.result = std::move(result);
    return i;
}
inline Instruction assignLabelAddress(std::string labelName, std::string result) {
    Instruction i;
    i.op = Op::AssignLabelAddress;
    i.arg0 = std::move(labelName);
    i.result = std::move(result);
    return i;
}
inline Instruction lvalueAssign(std::string operand, std::string result) {
    return detail::unary(Op::LvalueAssign, std::move(operand), std::move(result));
}
inline Instruction addressOf(std::string operand, std::string result) {
    return detail::unary(Op::AddressOf, std::move(operand), std::move(result));
}
inline Instruction dereference(std::string operand, std::string lvalue, std::string result) {
    Instruction i;
    i.op = Op::Dereference;
    i.arg0 = std::move(operand);
    i.arg1 = std::move(lvalue);
    i.result = std::move(result);
    return i;
}
inline Instruction indexAddress(std::string base, std::string index, int elementSizeBytes, std::string result,
        symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject) {
    Instruction i;
    i.op = Op::IndexAddress;
    i.arg0 = std::move(base);
    i.arg1 = std::move(index);
    i.result = std::move(result);
    i.imm = elementSizeBytes;
    i.baseMode = baseMode;
    return i;
}
inline Instruction fieldAddress(std::string base, int offsetBytes, std::string result,
        symbols::AddressBaseMode baseMode = symbols::AddressBaseMode::LeaObject) {
    Instruction i;
    i.op = Op::FieldAddress;
    i.arg0 = std::move(base);
    i.result = std::move(result);
    i.imm = offsetBytes;
    i.baseMode = baseMode;
    return i;
}
inline Instruction pointerOffset(std::string base, std::string index, int elementSizeBytes, std::string result,
        bool subtract) {
    Instruction i;
    i.op = Op::PointerOffset;
    i.arg0 = std::move(base);
    i.arg1 = std::move(index);
    i.result = std::move(result);
    i.imm = elementSizeBytes;
    i.pointerSubtract = subtract;
    return i;
}
inline Instruction pointerDiff(std::string left, std::string right, int elementSizeBytes, std::string result) {
    Instruction i;
    i.op = Op::PointerDiff;
    i.arg0 = std::move(left);
    i.arg1 = std::move(right);
    i.result = std::move(result);
    i.imm = elementSizeBytes;
    return i;
}
inline Instruction functionAddress(std::string functionName, std::string result) {
    return detail::unary(Op::FunctionAddress, std::move(functionName), std::move(result));
}
inline Instruction valueCompare(std::string left, std::string right) {
    Instruction i;
    i.op = Op::ValueCompare;
    i.arg0 = std::move(left);
    i.arg1 = std::move(right);
    return i;
}
inline Instruction zeroCompare(std::string symbol) {
    Instruction i;
    i.op = Op::ZeroCompare;
    i.arg0 = std::move(symbol);
    return i;
}
inline Instruction jump(std::string labelName, JumpCondition condition = JumpCondition::UNCONDITIONAL) {
    Instruction i;
    i.op = Op::Jump;
    i.arg0 = std::move(labelName);
    i.cond = condition;
    return i;
}
inline Instruction label(std::string name) {
    Instruction i;
    i.op = Op::Label;
    i.arg0 = std::move(name);
    return i;
}
inline Instruction argument(std::string name) {
    Instruction i;
    i.op = Op::Argument;
    i.arg0 = std::move(name);
    return i;
}
inline Instruction call(std::string procedureName, bool indirect = false) {
    Instruction i;
    i.op = Op::Call;
    i.arg0 = std::move(procedureName);
    i.callIndirect = indirect;
    return i;
}
inline Instruction retrieve(std::string resultName) {
    Instruction i;
    i.op = Op::Retrieve;
    i.result = std::move(resultName);
    return i;
}
inline Instruction ret(std::string returnSymbol) {
    Instruction i;
    i.op = Op::Return;
    i.arg0 = std::move(returnSymbol);
    return i;
}
inline Instruction voidReturn() {
    Instruction i;
    i.op = Op::VoidReturn;
    return i;
}

} // namespace ir
} // namespace codegen

#endif // CODEGEN_IR_BUILDERS_H_
