#include "InstructionSet.h"

#include <iostream>
#include <ostream>
#include <sstream>

namespace code_generator {

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

InstructionSet::InstructionSet() {
}

std::string InstructionSet::preamble() const {
    std::ostringstream oss;
    oss << "section .data\n" << "\tbuf db 255\n\n";
    oss << "section .text\n" << "\tglobal _start\n\n" << "___output:\n" << "\tpush eax\n" << "\tpush ebx\n"
            << "\tpush ecx\n" << "\tpush edx\n"
            << "\tpush ebp\n" << "\tmov ebp, esp\n" << "\tpush dword 10\n" << "\tmov eax, ecx\n" << "\tmov ecx, 4\n"
            << "\tmov ebx, eax\n" << "\txor edi, edi\n"
            << "\tand ebx, 0x80000000\n" << "\tjz ___loop\n" << "\tmov dword edi, 1\n" << "\tnot eax\n"
            << "\tadd dword eax, 1\n" << "\n___loop:\n"
            << "\tmov ebx, 10\n" << "\txor edx, edx\n" << "\tdiv ebx\n" << "\tadd edx, 0x30\n" << "\tpush edx\n"
            << "\tadd ecx, 4\n" << "\tcmp eax, 0\n"
            << "\tjg ___loop\n" << "\tcmp edi, 0\n" << "\tjz ___output_exit\n" << "\tadd ecx, 4\n"
            << "\tpush dword 45\n" << "___output_exit:\n"
            << "\tmov edx, ecx\n" << "\tmov ecx, esp\n" << "\tmov ebx, 1\n" << "\tmov eax, 4\n" << "\tint 0x80\n"
            << "\tmov esp, ebp\n" << "\tpop ebp\n"
            << "\tpop edx\n" << "\tpop ecx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";

    oss << "___input:\n" << "\tpush eax\n" << "\tpush ebx\n" << "\tpush edx\n" << "\tpush ebp\n"
            << "\tmov ebp, esp\n" << "\tmov ecx, buf\n"
            << "\tmov ebx, 0\n" << "\tmov edx, 255\n" << "\tmov eax, 3\n" << "\tint 0x80\n" << "\txor eax, eax\n"
            << "\txor ebx, ebx\n" << "\tmov ebx, 10\n"
            << "\txor edx, edx\n" << "___to_dec:\n" << "\tcmp byte [ecx], 10\n" << "\tje ___exit_input\n"
            << "\tmul ebx\n" << "\tmov dl, byte [ecx]\n"
            << "\tsub dl, 48\n" << "\tadd eax, edx\n" << "\tinc ecx\n" << "\tjmp ___to_dec\n" << "___exit_input:\n"
            << "\tmov ecx, eax\n" << "\tmov esp, ebp\n"
            << "\tpop ebp\n" << "\tpop edx\n" << "\tpop ebx\n" << "\tpop eax\n" << "\tret\n\n";
    return oss.str();
}

std::string InstructionSet::mainProcedure() const {
    return "_start:\n";
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

std::string InstructionSet::negate(const Register& reg) const {
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
    return "mov dword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + constant + "\n";
}

std::string InstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant + "\n";
}

std::string InstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    return "cmp " + leftArgument.getName() + ", " + "dword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string InstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName() + "\n";
}

std::string InstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    return "cmp dword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + rightArgument.getName() + "\n";
}

std::string InstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant) + "\n";
}

std::string InstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    return "cmp dword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + std::to_string(constant) + "\n";
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

std::string InstructionSet::interrupt(std::string interruptCode) const {
    return "int " + interruptCode + "\n";
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
    return "imul dword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName() + "\n";
}

std::string InstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    return "idiv dword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName() + "\n";
}

std::string InstructionSet::inc(const Register& operandBase, int operandOffset) const {
    return "inc dword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

std::string InstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName() + "\n";
}

std::string InstructionSet::dec(const Register& operandBase, int operandOffset) const {
    return "dec dword " + memoryOffsetMnemonic(operandBase, operandOffset) + "\n";
}

} /* namespace code_generator */
