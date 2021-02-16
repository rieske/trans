#include "ATandTInstructionSet.h"

#include "Register.h"

#include <stdexcept>

namespace {

using codegen::Register;

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return (memoryOffset ? "-" + std::to_string(memoryOffset) : "") + "(%" + memoryBase.getName() + ")";
}

std::string registerAccess(const Register& reg) {
    return "%" + reg.getName();
}

std::string constantReference(int constant) {
   return "$" + std::to_string(constant);
}

}

namespace codegen {


std::string ATandTInstructionSet::preamble() const {
    return ".extern scanf\n"
            ".extern printf\n\n"
            ".data\n"
            "sfmt: .string \"%d\"\n"
            "fmt: .string \"%d\n\"\n\n"
            ".text\n"
            ".globl main\n\n";
}

std::string ATandTInstructionSet::call(std::string procedureName) const {
    return "call " + procedureName;
}

std::string ATandTInstructionSet::push(const Register& reg) const {
    return "pushq " + registerAccess(reg);
}

std::string ATandTInstructionSet::pop(const Register& reg) const {
    return "popq " + registerAccess(reg);
}

std::string ATandTInstructionSet::add(const Register& reg, int constant) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::add(const Register& reg, int constant)" };
}

std::string ATandTInstructionSet::sub(const Register& reg, int constant) const {
    return "subq " + constantReference(constant) + ", " + registerAccess(reg);
}

std::string ATandTInstructionSet::lea(const Register& base, int offset, const Register& target) const {
    throw std::runtime_error { "not implemented ATandTinstructionSet::lea(const Register& base, int offset, const Register& target)"};
}

std::string ATandTInstructionSet::not_(const Register& reg) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::not_(const Register& reg)" };
}

std::string ATandTInstructionSet::mov(const Register& source, const Register& memoryBase, int memoryOffset) const {
    return "movq " + registerAccess(source) + ", " + memoryOffsetMnemonic(memoryBase, memoryOffset);
}

std::string ATandTInstructionSet::mov(const Register& source, const Register& destination) const {
    if (&source == &destination) {
        return "";
    }
    return "movq " + registerAccess(source) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(const Register& memoryBase, int memoryOffset, const Register& destination) const {
    return "movq " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset)" };
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& destination) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::mov(std::string constant, const Register& destination)" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset)" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument)" };
}

std::string ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument)" };
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(const Register& argument, int constant)" };
}

std::string ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant)" };
}

std::string ATandTInstructionSet::label(std::string name) const {
    return name + ":";
}

std::string ATandTInstructionSet::jmp(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jmp(std::string label)" };
}

std::string ATandTInstructionSet::je(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::je(std::string label)" };
}

std::string ATandTInstructionSet::jne(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jne(std::string label)" };
}

std::string ATandTInstructionSet::jg(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jg(std::string label)" };
}

std::string ATandTInstructionSet::jl(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jl(std::string label)" };
}

std::string ATandTInstructionSet::jge(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jge(std::string label)" };
}

std::string ATandTInstructionSet::jle(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jle(std::string label)" };
}

std::string ATandTInstructionSet::syscall() const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::syscall()" };
}

std::string ATandTInstructionSet::leave() const {
    return "leave";
}

std::string ATandTInstructionSet::ret() const {
    return "ret";
}

std::string ATandTInstructionSet::xor_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::xor_(const Register& operand, const Register& result) " };
}

std::string ATandTInstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result)" };
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::or_(const Register& operand, const Register& result)" };
}

std::string ATandTInstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result)" };
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::and_(const Register& operand, const Register& result)" };
}

std::string ATandTInstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error {
            "not implemented ATandTInstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result)" };
}

std::string ATandTInstructionSet::add(const Register& operand, const Register& result) const {
    return "addq " + registerAccess(operand) + ", " + registerAccess(result); // result = result + operand
}

std::string ATandTInstructionSet::add(const Register& operandBase, int operandOffset, const Register& result) const {
    return "addq " + memoryOffsetMnemonic(operandBase, operandOffset) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result) const {
    return "subq " + registerAccess(operand) + ", " + registerAccess(result); // result = result - operand
}

std::string ATandTInstructionSet::sub(const Register& operandBase, int operandOffset, const Register& result) const {
    return "subq " + memoryOffsetMnemonic(operandBase, operandOffset) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::imul(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::imul(const Register& operand)" };
}

std::string ATandTInstructionSet::imul(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::imul(const Register& operandBase, int operandOffset)" };
}

std::string ATandTInstructionSet::idiv(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::idiv(const Register& operand)" };
}

std::string ATandTInstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::idiv(const Register& operandBase, int operandOffset)" };
}

std::string ATandTInstructionSet::inc(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::inc(const Register& operand)" };
}

std::string ATandTInstructionSet::inc(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::inc(const Register& operandBase, int operandOffset)" };
}

std::string ATandTInstructionSet::dec(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::dec(const Register& operand)" };
}

std::string ATandTInstructionSet::dec(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::dec(const Register& operandBase, int operandOffset)" };
}

std::string ATandTInstructionSet::neg(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::neg(const Register& operand)" };
}

} // namespace codegen

