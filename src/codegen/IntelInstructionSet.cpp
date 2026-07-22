#include "IntelInstructionSet.h"

#include "Register.h"

#include <iostream>
#include <sstream>

namespace {

std::string memoryOffsetMnemonic(const codegen::Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

std::string memoryReference(const codegen::MemoryOperand& operand) {
    if (operand.isGlobal()) {
        return "[rel " + operand.label() + "]";
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

} // namespace

namespace codegen {

IntelInstructionSet::~IntelInstructionSet() = default;

// TODO: this needs to be rethought, expanded and unit tested separately
// currently just a spike for handling newlines and driven by functional tests
// - needs to handle all kinds of escape sequences
// - needs to handle single quotes - will break now if constant contains a single quote
std::string toConstantDeclaration(std::string escapedConstant) {
    auto constantValue = escapedConstant.substr(1, escapedConstant.length()-2); // strip "
    std::stringstream declaration;
    declaration << "db '";
    for (auto it = escapedConstant.cbegin()+1; it != escapedConstant.cend()-1; ++it) {
        if (*it == '\\' && *(it+1) == 'n') {
            declaration << "', 10, '";
            ++it;
        } else {
            declaration << *it;
        }
    }
    declaration << "', 0";
    return declaration.str();
}

std::string IntelInstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables) const {
    std::stringstream preamble;
    preamble << "default rel\n"
            "extern scanf\n"
            "extern printf\n\n"
            "section .data\n";
    for (const auto& constant : constants) {
        preamble << "\t" << constant.first << " " << toConstantDeclaration(constant.second) << "\n";
    }
    // One qword per global for now (sizeInBytes is tracked for StackMachine homes only).
    for (const auto& global : globalVariables) {
        preamble << "\t" << global.name << " dq " << global.initializerLiteral << "\n";
    }
    preamble << "\n"
            "section .text\n"
            "\tglobal main\n\n";
    return preamble.str();
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

std::string IntelInstructionSet::lea(const MemoryOperand& source, const Register& target) const {
    return "lea " + target.getName() + ", " + memoryReference(source);
}

std::string IntelInstructionSet::not_(const Register& reg) const {
    return "not " + reg.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const MemoryOperand& destination) const {
    return "mov " + memoryReference(destination) + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const Register& to) const {
    if (&from == &to) {
        return "";
    }
    return "mov " + to.getName() + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const MemoryOperand& source, const Register& to) const {
    return "mov " + to.getName() + ", " + memoryReference(source);
}

std::string IntelInstructionSet::mov(std::string constant, const MemoryOperand& destination) const {
    return "mov qword " + memoryReference(destination) + ", " + constant;
}

std::string IntelInstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant;
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + "qword " + memoryReference(rightArgument);
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const {
    return "cmp qword " + memoryReference(leftArgument) + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::cmp(const MemoryOperand& leftArgument, int constant) const {
    return "cmp qword " + memoryReference(leftArgument) + ", " + std::to_string(constant);
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

std::string IntelInstructionSet::xor_(const MemoryOperand& operand, const Register& result) const {
    return "xor " + result.getName() + ", " + memoryReference(operand);
}

std::string IntelInstructionSet::or_(const Register& operand, const Register& result) const {
    return "or " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::or_(const MemoryOperand& operand, const Register& result) const {
    return "or " + result.getName() + ", " + memoryReference(operand);
}

std::string IntelInstructionSet::and_(const Register& operand, const Register& result) const {
    return "and " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::and_(const MemoryOperand& operand, const Register& result) const {
    return "and " + result.getName() + ", " + memoryReference(operand);
}

std::string IntelInstructionSet::shl(const Register& result) const {
    return "shl " + result.getName() + ", cl";
}

//std::string IntelInstructionSet::shl(std::string constant, const Register& result) const {
//}

std::string IntelInstructionSet::shr(const Register& result) const {
    // Signed integer >> must arithmetic-shift (sign-extend); logical shr breaks
    // negatives, e.g. (~7)>>2 became a large positive instead of -2.
    return "sar " + result.getName() + ", cl";
}

//std::string IntelInstructionSet::shr(std::string constant, const Register& result) const {
//}

std::string IntelInstructionSet::add(const Register& operand, const Register& result) const {
    return "add " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::add(const MemoryOperand& operand, const Register& result) const {
    return "add " + result.getName() + ", " + memoryReference(operand);
}

std::string IntelInstructionSet::sub(const Register& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::sub(const MemoryOperand& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + memoryReference(operand);
}

std::string IntelInstructionSet::imul(const Register& operand) const {
    return "imul " + operand.getName();
}

std::string IntelInstructionSet::imul(const MemoryOperand& operand) const {
    return "imul qword " + memoryReference(operand);
}

std::string IntelInstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName();
}

std::string IntelInstructionSet::idiv(const MemoryOperand& operand) const {
    return "idiv qword " + memoryReference(operand);
}

std::string IntelInstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName();
}

std::string IntelInstructionSet::inc(const MemoryOperand& operand) const {
    return "inc qword " + memoryReference(operand);
}

std::string IntelInstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName();
}

std::string IntelInstructionSet::dec(const MemoryOperand& operand) const {
    return "dec qword " + memoryReference(operand);
}

std::string IntelInstructionSet::neg(const Register& operand) const {
    return "neg " + operand.getName();
}

} // namespace codegen

