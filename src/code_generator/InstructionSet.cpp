#include "InstructionSet.h"

namespace codegen {

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

InstructionSet::InstructionSet() {
}

std::string InstructionSet::preamble() const {
    return "extern scanf\n"
            "extern  printf\n\n"

            "section .data\n"
            "\tsfmt db '%d', 0\n"
            "\tfmt db '%d', 10, 0\n\n"

            "section .text\n"
            "\tglobal main\n\n";
}

std::string InstructionSet::mainProcedure() const {
    return "main:\n";
}

std::string InstructionSet::label(std::string name) const {
    return name + ":\n";
}

std::string InstructionSet::push(const Register& reg) const {
    return "push " + reg.getName() + "\n";
}

std::string InstructionSet::pop(const Register& reg) const {
    return "pop " + reg.getName() + "\n";
}

std::string InstructionSet::add(const Register& reg, int constant) const {
    return "add " + reg.getName() + ", " + std::to_string(constant) + "\n";
}

std::string InstructionSet::sub(const Register& reg, int constant) const {
    return "sub " + reg.getName() + ", " + std::to_string(constant) + "\n";
}

std::string InstructionSet::not_(const Register& reg) const {
    return "not " + reg.getName() + "\n";
}

std::string InstructionSet::mov(const Register& from, const Register& memoryBase, int memoryOffset) const {
    return "mov " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + from.getName() + "\n";
}

std::string InstructionSet::mov(const Register& from, const Register& to) const {
    return "mov " + to.getName() + ", " + from.getName() + "\n";
}

std::string InstructionSet::mov(const Register& memoryBase, int memoryOffset, const Register& to) const {
    return "mov " + to.getName() + ", " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string InstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    return "mov qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + constant + "\n";
}

std::string InstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant + "\n";
}

std::string InstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    return "cmp " + leftArgument.getName() + ", " + "qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string InstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName() + "\n";
}

std::string InstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + rightArgument.getName() + "\n";
}

std::string InstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant) + "\n";
}

std::string InstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + std::to_string(constant) + "\n";
}

std::string InstructionSet::call(std::string procedureName) const {
    return "call " + procedureName + "\n";
}

std::string InstructionSet::jmp(std::string label) const {
    return "jmp " + label + "\n";
}

std::string InstructionSet::je(std::string label) const {
    return "je " + label + "\n";
}

std::string InstructionSet::jne(std::string label) const {
    return "jne " + label + "\n";
}

std::string InstructionSet::jg(std::string label) const {
    return "jg " + label + "\n";
}

std::string InstructionSet::jl(std::string label) const {
    return "jl " + label + "\n";
}

std::string InstructionSet::jge(std::string label) const {
    return "jge " + label + "\n";
}

std::string InstructionSet::jle(std::string label) const {
    return "jle " + label + "\n";
}

std::string InstructionSet::syscall() const {
    return "syscall\n";
}

std::string InstructionSet::leave() const {
    return "leave\n";
}

std::string InstructionSet::ret() const {
    return "ret\n";
}

std::string InstructionSet::xor_(const Register& operand, const Register& result) const {
    return "xor " + result.getName() + ", " + operand.getName() + "\n";
}

std::string InstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "xor " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::or_(const Register& operand, const Register& result) const {
    return "or " + result.getName() + ", " + operand.getName() + "\n";
}

std::string InstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "or " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::and_(const Register& operand, const Register& result) const {
    return "and " + result.getName() + ", " + operand.getName() + "\n";
}

std::string InstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "and " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::add(const Register& operand, const Register& result) const {
    return "add " + result.getName() + ", " + operand.getName() + "\n";
}

std::string InstructionSet::add(const Register& operandBase, int operandOffset, const Register& result) const {
    return "add " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::sub(const Register& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + operand.getName() + "\n";
}

std::string InstructionSet::sub(const Register& operandBase, int operandOffset, const Register& result) const {
    return "sub " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::imul(const Register& operand) const {
    return "imul " + operand.getName() + "\n";
}

std::string InstructionSet::imul(const Register& operandBase, int operandOffset) const {
    return "imul qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName() + "\n";
}

std::string InstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    return "idiv qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName() + "\n";
}

std::string InstructionSet::inc(const Register& operandBase, int operandOffset) const {
    return "inc qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName() + "\n";
}

std::string InstructionSet::dec(const Register& operandBase, int operandOffset) const {
    return "dec qword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::neg(const Register& operand) const {
    return "neg " + operand.getName() + "\n";
}

} /* namespace code_generator */
