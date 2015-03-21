#include "InstructionSet.h"

namespace code_generator {

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

InstructionSet::InstructionSet() {
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
    return "mov " + to.getName() + " " + memoryOffsetMnemonic(memoryBase, memoryOffset) + "\n";
}

std::string InstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    return "mov " + memoryOffsetMnemonic(memoryBase, memoryOffset) + " " + constant + "\n";
}

std::string InstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + " " + constant + "\n";
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

} /* namespace code_generator */
