#include "ATandTInstructionSet.h"

#include "Register.h"
#include "MemoryOperand.h"

#include <stdexcept>

namespace {

using codegen::Register;

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    return (memoryOffset ? "-" + std::to_string(memoryOffset) : "") + "(%" + memoryBase.getName() + ")";
}

std::string memoryReference(const codegen::MemoryOperand& operand) {
    if (operand.isGlobal()) {
        throw std::runtime_error { "not implemented ATandTInstructionSet: global variable operand" };
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

std::string registerAccess(const Register& reg) {
    return "%" + reg.getName();
}

std::string constantReference(int constant) {
   return "$" + std::to_string(constant);
}

}

namespace codegen {

ATandTInstructionSet::~ATandTInstructionSet() = default;

std::string ATandTInstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) const {
    (void)constants;
    (void)globalVariables;
    (void)externalFunctions;
    (void)definedFunctions;
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
    return "addq " + constantReference(constant) + ", " + registerAccess(reg);
}

std::string ATandTInstructionSet::sub(const Register& reg, int constant) const {
    return "subq " + constantReference(constant) + ", " + registerAccess(reg);
}

std::string ATandTInstructionSet::lea(const MemoryOperand& source, const Register& target) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::lea" };
}

std::string ATandTInstructionSet::not_(const Register& reg) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::not_(const Register& reg)" };
}

std::string ATandTInstructionSet::mov(const Register& source, const MemoryOperand& destination) const {
    return "movq " + registerAccess(source) + ", " + memoryReference(destination);
}

std::string ATandTInstructionSet::mov(const Register& source, const Register& destination) const {
    if (&source == &destination) {
        return "";
    }
    return "movq " + registerAccess(source) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(const MemoryOperand& source, const Register& destination) const {
    return "movq " + memoryReference(source) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(std::string constant, const MemoryOperand& destination) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::mov(std::string constant, MemoryOperand)" };
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& destination) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::mov(std::string constant, const Register& destination)" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(Register, MemoryOperand)" };
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument)" };
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(MemoryOperand, Register)" };
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(const Register& argument, int constant)" };
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, int constant) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::cmp(MemoryOperand, int constant)" };
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

std::string ATandTInstructionSet::ja(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::ja(std::string label)" };
}

std::string ATandTInstructionSet::jb(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jb(std::string label)" };
}

std::string ATandTInstructionSet::jae(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jae(std::string label)" };
}

std::string ATandTInstructionSet::jbe(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jbe(std::string label)" };
}

std::string ATandTInstructionSet::jge(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jge(std::string label)" };
}

std::string ATandTInstructionSet::jle(std::string label) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::jle(std::string label)" };
}


std::string ATandTInstructionSet::leave() const {
    return "leave";
}

std::string ATandTInstructionSet::ret() const {
    return "ret";
}

std::string ATandTInstructionSet::xor_(const Register& operand, const Register& result) const {
    return "xorq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::xor_(const MemoryOperand& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::xor_(MemoryOperand, Register)" };
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::or_(const Register& operand, const Register& result)" };
}

std::string ATandTInstructionSet::or_(const MemoryOperand& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::or_(MemoryOperand, Register)" };
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::and_(const Register& operand, const Register& result)" };
}

std::string ATandTInstructionSet::and_(const MemoryOperand& operand, const Register& result) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::and_(MemoryOperand, Register)" };
}

std::string ATandTInstructionSet::shl(const Register& result) const {
    throw std::runtime_error {
        "not implemented ATandTInstructionSet::shl(const Register& operand, const Register& result)"
    };
}


std::string ATandTInstructionSet::shr(const Register& result) const {
    throw std::runtime_error {
        "not implemented ATandTInstructionSet::shr(const Register& operand, const Register& result)"
    };
}

std::string ATandTInstructionSet::sar(const Register& result) const {
    throw std::runtime_error {
        "not implemented ATandTInstructionSet::sar(const Register& operand, const Register& result)"
    };
}


std::string ATandTInstructionSet::add(const Register& operand, const Register& result) const {
    return "addq " + registerAccess(operand) + ", " + registerAccess(result); // result = result + operand
}

std::string ATandTInstructionSet::add(const MemoryOperand& operand, const Register& result) const {
    return "addq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result) const {
    return "subq " + registerAccess(operand) + ", " + registerAccess(result); // result = result - operand
}

std::string ATandTInstructionSet::sub(const MemoryOperand& operand, const Register& result) const {
    return "subq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::imul(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::imul(const Register& operand)" };
}

std::string ATandTInstructionSet::imul(const MemoryOperand& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::imul(MemoryOperand)" };
}

std::string ATandTInstructionSet::idiv(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::idiv(const Register& operand)" };
}

std::string ATandTInstructionSet::idiv(const MemoryOperand& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::idiv(MemoryOperand)" };
}

std::string ATandTInstructionSet::div(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::div(const Register& operand)" };
}

std::string ATandTInstructionSet::div(const MemoryOperand& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::div(MemoryOperand)" };
}

std::string ATandTInstructionSet::inc(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::inc(const Register& operand)" };
}

std::string ATandTInstructionSet::inc(const MemoryOperand& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::inc(MemoryOperand)" };
}

std::string ATandTInstructionSet::dec(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::dec(const Register& operand)" };
}

std::string ATandTInstructionSet::dec(const MemoryOperand& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::dec(MemoryOperand)" };
}

std::string ATandTInstructionSet::neg(const Register& operand) const {
    throw std::runtime_error { "not implemented ATandTInstructionSet::neg(const Register& operand)" };
}

} // namespace codegen

