#include "codegen/ATandTInstructionSet.h"

#include "Register.h"

#include <stdexcept>
#include <string>

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
    return "call " + procedureName + "\n";
}

std::string ATandTInstructionSet::push(const Register& reg) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::pop(const Register& reg) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::add(const Register& reg, int constant) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::sub(const Register& reg, int constant) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::not_(const Register& reg) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::mov(const Register& from, const Register& memoryBase, int memoryOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::mov(const Register& from, const Register& to) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::mov(const Register& memoryBase, int memoryOffset, const Register& to) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& to) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::label(std::string name) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jmp(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::je(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jne(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jg(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jl(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jge(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::jle(std::string label) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::syscall() const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::leave() const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::ret() const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::xor_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::add(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::add(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::sub(const Register& operandBase, int operandOffset, const Register& result) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::imul(const Register& operand) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::imul(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::idiv(const Register& operand) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::inc(const Register& operand) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::inc(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::dec(const Register& operand) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::dec(const Register& operandBase, int operandOffset) const {
    throw std::runtime_error { "not implemented" };
}

std::string ATandTInstructionSet::neg(const Register& operand) const {
    throw std::runtime_error { "not implemented" };
}

} /* namespace codegen */
