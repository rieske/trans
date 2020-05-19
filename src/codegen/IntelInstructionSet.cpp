#include "IntelInstructionSet.h"

#include "Register.h"

namespace {

using codegen::Register;

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

}

namespace codegen {

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
    return name + ":";
}

std::string IntelInstructionSet::push(const Register& reg) const {
    return "push " + reg.getName();
}

std::string IntelInstructionSet::pop(const Register& reg) const {
    return "pop " + reg.getName();
}

std::string IntelInstructionSet::add(const Register& reg, int constant) const {
    return "add " + reg.getName() + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::sub(const Register& reg, int constant) const {
    return "sub " + reg.getName() + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::not_(const Register& reg) const {
    return "not " + reg.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const Register& memoryBase, int memoryOffset) const {
    return "mov " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const Register& to) const {
    if (&from == &to) {
        return "";
    }
    return "mov " + to.getName() + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const Register& memoryBase, int memoryOffset, const Register& to) const {
    return "mov " + to.getName() + ", " + memoryOffsetMnemonic(memoryBase, memoryOffset);
}

std::string IntelInstructionSet::mov(std::string constant, const Register& memoryBase, int memoryOffset) const {
    return "mov qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + constant;
}

std::string IntelInstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant;
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& memoryBase, int memoryOffset) const {
    return "cmp " + leftArgument.getName() + ", " + "qword " + memoryOffsetMnemonic(memoryBase, memoryOffset);
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const Register& memoryBase, int memoryOffset, const Register& rightArgument) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::cmp(const Register& memoryBase, int memoryOffset, int constant) const {
    return "cmp qword " + memoryOffsetMnemonic(memoryBase, memoryOffset) + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::call(std::string procedureName) const {
    return "call " + procedureName;
}

std::string IntelInstructionSet::jmp(std::string label) const {
    return "jmp " + label;
}

std::string IntelInstructionSet::je(std::string label) const {
    return "je " + label;
}

std::string IntelInstructionSet::jne(std::string label) const {
    return "jne " + label;
}

std::string IntelInstructionSet::jg(std::string label) const {
    return "jg " + label;
}

std::string IntelInstructionSet::jl(std::string label) const {
    return "jl " + label;
}

std::string IntelInstructionSet::jge(std::string label) const {
    return "jge " + label;
}

std::string IntelInstructionSet::jle(std::string label) const {
    return "jle " + label;
}

std::string IntelInstructionSet::syscall() const {
    return "syscall";
}

std::string IntelInstructionSet::leave() const {
    return "leave";
}

std::string IntelInstructionSet::ret() const {
    return "ret";
}

std::string IntelInstructionSet::xor_(const Register& operand, const Register& result) const {
    return "xor " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::xor_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "xor " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::or_(const Register& operand, const Register& result) const {
    return "or " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::or_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "or " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::and_(const Register& operand, const Register& result) const {
    return "and " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::and_(const Register& operandBase, int operandOffset, const Register& result) const {
    return "and " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::add(const Register& operand, const Register& result) const {
    return "add " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::add(const Register& operandBase, int operandOffset, const Register& result) const {
    return "add " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::sub(const Register& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::sub(const Register& operandBase, int operandOffset, const Register& result) const {
    return "sub " + result.getName() + ", " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::imul(const Register& operand) const {
    return "imul " + operand.getName();
}

std::string IntelInstructionSet::imul(const Register& operandBase, int operandOffset) const {
    return "imul qword " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName();
}

std::string IntelInstructionSet::idiv(const Register& operandBase, int operandOffset) const {
    return "idiv qword " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName();
}

std::string IntelInstructionSet::inc(const Register& operandBase, int operandOffset) const {
    return "inc qword " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName();
}

std::string IntelInstructionSet::dec(const Register& operandBase, int operandOffset) const {
    return "dec qword " + memoryOffsetMnemonic(operandBase, operandOffset);
}

std::string IntelInstructionSet::neg(const Register& operand) const {
    return "neg " + operand.getName();
}

} /* namespace code_generator */
