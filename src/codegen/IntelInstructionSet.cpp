#include "IntelInstructionSet.h"

#include "Register.h"

namespace codegen {

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

std::string IntelInstructionSet::preamble() const {
    return "extern scanf\n"
            "extern  printf\n\n"

            "section .data\n"
            "\tsfmt db '%d', 0\n"
            "\tfmt db '%d', 10, 0\n\n"

            "section .text\n"
            "\tglobal main\n\n";
}

std::string IntelInstructionSet::label(std::string name) const {
    return name + ":\n";
}

std::string IntelInstructionSet::push(const Register& reg) const {
    return "push " + reg.getName() + "\n";
}

std::string IntelInstructionSet::pop(const Register& reg) const {
    return "pop " + reg.getName() + "\n";
}

std::string IntelInstructionSet::add(const Register& reg, int constant) const {
    return "add " + reg.getName() + ", " + std::to_string(constant) + "\n";
}

std::string IntelInstructionSet::sub(const Register& reg, int constant) const {
    return "sub " + reg.getName() + ", " + std::to_string(constant) + "\n";
}

std::string IntelInstructionSet::not_(const Register& reg) const {
    return "not " + reg.getName() + "\n";
}

std::string IntelInstructionSet::mov(const Register& from, const Register& memoryBase, int memoryOffset) const {
    return "mov " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + from.getName() + "\n";
}

std::string IntelInstructionSet::mov(const Register& from, const Register& to) const {
    return "mov " + to.getName() + ", " + from.getName() + "\n";
}

std::string IntelInstructionSet::mov(const Register& memoryBase, int memoryOffset, const Register& to) const {
    return "mov " + to.getName() + ", " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string IntelInstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    return "mov qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + constant + "\n";
}

std::string IntelInstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant + "\n";
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    return "cmp " + leftArgument.getName() + ", " + "qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName() + "\n";
}

std::string IntelInstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + rightArgument.getName() + "\n";
}

std::string IntelInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant) + "\n";
}

std::string IntelInstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + std::to_string(constant) + "\n";
}

std::string IntelInstructionSet::call(std::string procedureName) const {
    return "call " + procedureName + "\n";
}

std::string IntelInstructionSet::jmp(std::string label) const {
    return "jmp " + label + "\n";
}

std::string IntelInstructionSet::je(std::string label) const {
    return "je " + label + "\n";
}

std::string IntelInstructionSet::jne(std::string label) const {
    return "jne " + label + "\n";
}

std::string IntelInstructionSet::jg(std::string label) const {
    return "jg " + label + "\n";
}

std::string IntelInstructionSet::jl(std::string label) const {
    return "jl " + label + "\n";
}

std::string IntelInstructionSet::jge(std::string label) const {
    return "jge " + label + "\n";
}

std::string IntelInstructionSet::jle(std::string label) const {
    return "jle " + label + "\n";
}

std::string IntelInstructionSet::syscall() const {
    return "syscall\n";
}

std::string IntelInstructionSet::leave() const {
    return "leave\n";
}

std::string IntelInstructionSet::ret() const {
    return "ret\n";
}

std::string IntelInstructionSet::xor_(const Register& operand, const Register& result) const {
    return "xor " + result.getName() + ", " + operand.getName() + "\n";
}

std::string IntelInstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "xor " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::or_(const Register& operand, const Register& result) const {
    return "or " + result.getName() + ", " + operand.getName() + "\n";
}

std::string IntelInstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "or " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::and_(const Register& operand, const Register& result) const {
    return "and " + result.getName() + ", " + operand.getName() + "\n";
}

std::string IntelInstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "and " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::add(const Register& operand, const Register& result) const {
    return "add " + result.getName() + ", " + operand.getName() + "\n";
}

std::string IntelInstructionSet::add(const Register& operandBase, int operandOffset, const Register& result) const {
    return "add " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::sub(const Register& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + operand.getName() + "\n";
}

std::string IntelInstructionSet::sub(const Register& operandBase, int operandOffset, const Register& result) const {
    return "sub " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::imul(const Register& operand) const {
    return "imul " + operand.getName() + "\n";
}

std::string IntelInstructionSet::imul(const Register& operandBase, int operandOffset) const {
    return "imul qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName() + "\n";
}

std::string IntelInstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    return "idiv qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName() + "\n";
}

std::string IntelInstructionSet::inc(const Register& operandBase, int operandOffset) const {
    return "inc qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName() + "\n";
}

std::string IntelInstructionSet::dec(const Register& operandBase, int operandOffset) const {
    return "dec qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string IntelInstructionSet::neg(const Register& operand) const {
    return "neg " + operand.getName() + "\n";
}

} /* namespace code_generator */
