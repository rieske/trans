#include "ATandTInstructionSet.h"

#include "Register.h"
#include "RegisterSubreg.h"
#include "MemoryOperand.h"

#include <sstream>

namespace {

using codegen::Register;

std::string registerAccess(const std::string& name) {
    return "%" + name;
}

std::string registerAccess(const Register& reg) {
    return registerAccess(reg.getName());
}

std::string memoryOffsetMnemonic(const Register& memoryBase, int memoryOffset) {
    if (memoryOffset == 0) {
        return "(%" + memoryBase.getName() + ")";
    }
    return std::to_string(memoryOffset) + "(%" + memoryBase.getName() + ")";
}

std::string memoryReference(const codegen::MemoryOperand& operand) {
    if (operand.isGlobal()) {
        return operand.label() + "(%rip)";
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

std::string constantReference(int constant) {
    return "$" + std::to_string(constant);
}

std::string immediate(const std::string& constant) {
    if (!constant.empty() && constant[0] == '$') {
        return constant;
    }
    return "$" + constant;
}

std::string toGasStringDirective(const std::string& escapedConstant) {
    if (escapedConstant.size() >= 2 && escapedConstant.front() == '"' && escapedConstant.back() == '"') {
        return ".string " + escapedConstant;
    }
    return ".string \"" + escapedConstant + "\"";
}

} // namespace

namespace codegen {

ATandTInstructionSet::~ATandTInstructionSet() = default;

std::string ATandTInstructionSet::globl(const std::string& name) const {
    return ".globl " + name;
}

std::string ATandTInstructionSet::externDirective(const std::string& name) const {
    return ".extern " + name;
}

std::string ATandTInstructionSet::dataSectionHeader() const {
    return "\n.section .data\n";
}

std::string ATandTInstructionSet::textSectionHeader() const {
    return "\n.section .text\n\n";
}

std::string ATandTInstructionSet::constantLine(const std::string& name, const std::string& escapedValue) const {
    return name + ":\n\t" + toGasStringDirective(escapedValue) + "\n";
}

std::string ATandTInstructionSet::dataObjectLines(const GlobalVariable& global) const {
    const auto operands = global.dataOperands();
    if (global.emitAsDword()) {
        return global.name + ":\n\t.long "
                + (operands.empty() ? "0" : operands.front()) + "\n";
    }
    std::stringstream out;
    out << global.name << ":\n\t.quad ";
    for (std::size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << operands[i];
    }
    out << "\n";
    return out.str();
}

std::string ATandTInstructionSet::call(std::string procedureName) const {
    return "call " + procedureName;
}

std::string ATandTInstructionSet::callPlt(std::string procedureName) const {
    return "call " + procedureName + "@plt";
}

std::string ATandTInstructionSet::callIndirect(const Register& target) const {
    return "call *" + registerAccess(target);
}

std::string ATandTInstructionSet::loadGot(std::string symbolName, const Register& target) const {
    return "movq " + symbolName + "@GOTPCREL(%rip), " + registerAccess(target);
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
    return "leaq " + memoryReference(source) + ", " + registerAccess(target);
}

std::string ATandTInstructionSet::not_(const Register& reg) const {
    return "notq " + registerAccess(reg);
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
    return "movq " + immediate(constant) + ", " + memoryReference(destination);
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& destination) const {
    return "movq " + immediate(constant) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const {
    return "cmpq " + memoryReference(rightArgument) + ", " + registerAccess(leftArgument);
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmpq " + registerAccess(rightArgument) + ", " + registerAccess(leftArgument);
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const {
    return "cmpq " + registerAccess(rightArgument) + ", " + memoryReference(leftArgument);
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmpq " + constantReference(constant) + ", " + registerAccess(argument);
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, int constant) const {
    return "cmpq " + constantReference(constant) + ", " + memoryReference(leftArgument);
}

std::string ATandTInstructionSet::label(std::string name) const {
    return name + ":";
}

std::string ATandTInstructionSet::jmp(std::string label) const {
    return "jmp " + label;
}

std::string ATandTInstructionSet::je(std::string label) const {
    return "je " + label;
}

std::string ATandTInstructionSet::jne(std::string label) const {
    return "jne " + label;
}

std::string ATandTInstructionSet::jg(std::string label) const {
    return "jg " + label;
}

std::string ATandTInstructionSet::jl(std::string label) const {
    return "jl " + label;
}

std::string ATandTInstructionSet::jge(std::string label) const {
    return "jge " + label;
}

std::string ATandTInstructionSet::jle(std::string label) const {
    return "jle " + label;
}

std::string ATandTInstructionSet::syscall() const {
    return "syscall";
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
    return "xorq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result) const {
    return "orq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::or_(const MemoryOperand& operand, const Register& result) const {
    return "orq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result) const {
    return "andq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::and_(const MemoryOperand& operand, const Register& result) const {
    return "andq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::shl(const Register& result) const {
    return "shlq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::shr(const Register& result) const {
    return "sarq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::add(const Register& operand, const Register& result) const {
    return "addq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::add(const MemoryOperand& operand, const Register& result) const {
    return "addq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result) const {
    return "subq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const MemoryOperand& operand, const Register& result) const {
    return "subq " + memoryReference(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::imul(const Register& operand) const {
    return "imulq " + registerAccess(operand);
}

std::string ATandTInstructionSet::imul(const MemoryOperand& operand) const {
    return "imulq " + memoryReference(operand);
}

std::string ATandTInstructionSet::idiv(const Register& operand) const {
    return "idivq " + registerAccess(operand);
}

std::string ATandTInstructionSet::idiv(const MemoryOperand& operand) const {
    return "idivq " + memoryReference(operand);
}

std::string ATandTInstructionSet::cqo() const {
    return "cqto";
}

std::string ATandTInstructionSet::inc(const Register& operand) const {
    return "incq " + registerAccess(operand);
}

std::string ATandTInstructionSet::inc(const MemoryOperand& operand) const {
    return "incq " + memoryReference(operand);
}

std::string ATandTInstructionSet::dec(const Register& operand) const {
    return "decq " + registerAccess(operand);
}

std::string ATandTInstructionSet::dec(const MemoryOperand& operand) const {
    return "decq " + memoryReference(operand);
}

std::string ATandTInstructionSet::neg(const Register& operand) const {
    return "negq " + registerAccess(operand);
}

std::string ATandTInstructionSet::movqGprToXmm(const Register& gpr, int xmmIndex) const {
    return "movq %" + gpr.getName() + ", %xmm" + std::to_string(xmmIndex);
}

std::string ATandTInstructionSet::movqXmmToGpr(int xmmIndex, const Register& gpr) const {
    return "movq %xmm" + std::to_string(xmmIndex) + ", %" + gpr.getName();
}

std::string ATandTInstructionSet::movdGprToXmm(const Register& gpr, int xmmIndex) const {
    return "movd %" + lowDwordName(gpr) + ", %xmm" + std::to_string(xmmIndex);
}

std::string ATandTInstructionSet::movdXmmToGpr(int xmmIndex, const Register& gpr) const {
    return "movd %xmm" + std::to_string(xmmIndex) + ", %" + lowDwordName(gpr);
}

std::string ATandTInstructionSet::movDword(const MemoryOperand& source, const Register& dest) const {
    return "movl " + memoryReference(source) + ", %" + lowDwordName(dest);
}

std::string ATandTInstructionSet::movDword(const Register& source, const MemoryOperand& dest) const {
    return "movl %" + lowDwordName(source) + ", " + memoryReference(dest);
}

std::string ATandTInstructionSet::cvtsi2sd(const Register& gpr, int xmmIndex) const {
    return "cvtsi2sdq %" + gpr.getName() + ", %xmm" + std::to_string(xmmIndex);
}

std::string ATandTInstructionSet::cvttsd2si(int xmmIndex, const Register& gpr) const {
    return "cvttsd2si %xmm" + std::to_string(xmmIndex) + ", %" + gpr.getName();
}

std::string ATandTInstructionSet::cvtsi2ss(const Register& gpr, int xmmIndex) const {
    return "cvtsi2ssq %" + gpr.getName() + ", %xmm" + std::to_string(xmmIndex);
}

std::string ATandTInstructionSet::cvttss2si(int xmmIndex, const Register& gpr) const {
    return "cvttss2si %xmm" + std::to_string(xmmIndex) + ", %" + gpr.getName();
}

std::string ATandTInstructionSet::cvtss2sd(int srcXmm, int dstXmm) const {
    return "cvtss2sd %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::cvtsd2ss(int srcXmm, int dstXmm) const {
    return "cvtsd2ss %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::addsd(int dstXmm, int srcXmm) const {
    return "addsd %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::subsd(int dstXmm, int srcXmm) const {
    return "subsd %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::mulsd(int dstXmm, int srcXmm) const {
    return "mulsd %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::divsd(int dstXmm, int srcXmm) const {
    return "divsd %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::addss(int dstXmm, int srcXmm) const {
    return "addss %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::subss(int dstXmm, int srcXmm) const {
    return "subss %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::mulss(int dstXmm, int srcXmm) const {
    return "mulss %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::divss(int dstXmm, int srcXmm) const {
    return "divss %xmm" + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::loadByteSignExtend(const Register& address, const Register& dest) const {
    return "movsbq (%" + address.getName() + "), " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadByteZeroExtend(const Register& address, const Register& dest) const {
    return "movzbq (%" + address.getName() + "), " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadWordSignExtend(const Register& address, const Register& dest) const {
    return "movswq (%" + address.getName() + "), " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadWordZeroExtend(const Register& address, const Register& dest) const {
    return "movzwq (%" + address.getName() + "), " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadDwordSignExtend(const Register& address, const Register& dest) const {
    return "movslq (%" + address.getName() + "), " + registerAccess(dest);
}

std::string ATandTInstructionSet::storeByte(const Register& source, const Register& address) const {
    return "movb %" + lowByteName(source) + ", (%" + address.getName() + ")";
}

std::string ATandTInstructionSet::storeDword(const Register& source, const Register& address) const {
    return "movl %" + lowDwordName(source) + ", (%" + address.getName() + ")";
}

} // namespace codegen
