#include "IntelInstructionSet.h"

#include "Register.h"
#include "RegisterSubreg.h"
#include "util/StringLiteralDecode.h"

#include <iostream>

namespace {

std::string memoryOffsetMnemonic(const codegen::Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

std::string memoryReference(const codegen::MemoryOperand& operand, const codegen::InstructionSet& isa) {
    if (operand.isGlobal()) {
        return "[rel " + isa.asmSymbol(operand.label()) + "]";
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

} // namespace

namespace codegen {

IntelInstructionSet::~IntelInstructionSet() = default;

std::string IntelInstructionSet::asmSymbol(const std::string& name) const {
    // $foo is the symbol foo; $ does not change the ELF name.
    if (name.empty() || name[0] == '$') {
        return name;
    }
    return "$" + name;
}

std::string IntelInstructionSet::globl(const std::string& name) const {
    return "global " + name;
}

std::string IntelInstructionSet::externDirective(const std::string& name) const {
    return "extern " + name;
}

std::string IntelInstructionSet::preamblePrefix() const {
    return "default rel\n";
}

std::string IntelInstructionSet::globlDataLine(const std::string& name) const {
    return "\t" + globl(name) + "\n";
}

std::string IntelInstructionSet::dataSectionHeader() const {
    return "\nsection .data\n";
}

std::string IntelInstructionSet::textSectionHeader() const {
    return "\nsection .text\n\n";
}

std::string IntelInstructionSet::constantLine(const std::string& name, const std::string& escapedValue) const {
    return "\t" + asmSymbol(name) + " " + util::toNasmDbDirective(escapedValue) + "\n";
}

std::string IntelInstructionSet::dataObjectLines(const GlobalVariable& global) const {
    if (global.emitAsDword()) {
        const auto values = global.initValuesOrZeros();
        const std::string operand = values.empty() ? "0" : dataOperandText(values.front());
        return "\t" + asmSymbol(global.name) + " dd " + operand + "\n";
    }
    const std::string operands = joinedDataOperands(global);
    return "\t" + asmSymbol(global.name) + " dq " + (operands.empty() ? "0" : operands) + "\n";
}

std::string IntelInstructionSet::label(std::string name) const {
    return asmSymbol(name) + ":";
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
    return "lea " + target.getName() + ", " + memoryReference(source, *this);
}

std::string IntelInstructionSet::not_(const Register& reg) const {
    return "not " + reg.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const MemoryOperand& destination) const {
    return "mov " + memoryReference(destination, *this) + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const Register& from, const Register& to) const {
    if (&from == &to) {
        return "";
    }
    return "mov " + to.getName() + ", " + from.getName();
}

std::string IntelInstructionSet::mov(const MemoryOperand& source, const Register& to) const {
    return "mov " + to.getName() + ", " + memoryReference(source, *this);
}

std::string IntelInstructionSet::mov(std::string constant, const MemoryOperand& destination) const {
    return "mov qword " + memoryReference(destination, *this) + ", " + constant;
}

std::string IntelInstructionSet::mov(std::string constant, const Register& to) const {
    return "mov " + to.getName() + ", " + constant;
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + "qword " + memoryReference(rightArgument, *this);
}

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmp " + leftArgument.getName() + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const {
    return "cmp qword " + memoryReference(leftArgument, *this) + ", " + rightArgument.getName();
}

std::string IntelInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmp " + argument.getName() + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::cmp(const MemoryOperand& leftArgument, int constant) const {
    return "cmp qword " + memoryReference(leftArgument, *this) + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::call(std::string procedureName) const {
    return "call " + asmSymbol(procedureName);
}

std::string IntelInstructionSet::callPlt(std::string procedureName) const {
    return "call " + asmSymbol(procedureName) + " wrt ..plt";
}

std::string IntelInstructionSet::callIndirect(const Register& target) const {
    return "call " + target.getName();
}

std::string IntelInstructionSet::loadGot(std::string symbolName, const Register& target) const {
    return "mov " + target.getName() + ", [rel " + asmSymbol(symbolName) + " wrt ..got]";
}

std::string IntelInstructionSet::jmp(std::string label) const {
    return "jmp " + asmSymbol(label);
}

std::string IntelInstructionSet::je(std::string label) const {
    return "je " + asmSymbol(label);
}

std::string IntelInstructionSet::jne(std::string label) const {
    return "jne " + asmSymbol(label);
}

std::string IntelInstructionSet::jg(std::string label) const {
    return "jg " + asmSymbol(label);
}

std::string IntelInstructionSet::jl(std::string label) const {
    return "jl " + asmSymbol(label);
}

std::string IntelInstructionSet::jge(std::string label) const {
    return "jge " + asmSymbol(label);
}

std::string IntelInstructionSet::jle(std::string label) const {
    return "jle " + asmSymbol(label);
}

std::string IntelInstructionSet::ja(std::string label) const {
    return "ja " + asmSymbol(label);
}

std::string IntelInstructionSet::jb(std::string label) const {
    return "jb " + asmSymbol(label);
}

std::string IntelInstructionSet::jae(std::string label) const {
    return "jae " + asmSymbol(label);
}

std::string IntelInstructionSet::jbe(std::string label) const {
    return "jbe " + asmSymbol(label);
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
    return "xor " + result.getName() + ", " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::or_(const Register& operand, const Register& result) const {
    return "or " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::or_(const MemoryOperand& operand, const Register& result) const {
    return "or " + result.getName() + ", " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::and_(const Register& operand, const Register& result) const {
    return "and " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::and_(const MemoryOperand& operand, const Register& result) const {
    return "and " + result.getName() + ", " + memoryReference(operand, *this);
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

std::string IntelInstructionSet::lshr(const Register& result) const {
    return "shr " + result.getName() + ", cl";
}

std::string IntelInstructionSet::shld(const Register& source, const Register& dest) const {
    return "shld " + dest.getName() + ", " + source.getName() + ", cl";
}

std::string IntelInstructionSet::shrd(const Register& source, const Register& dest) const {
    return "shrd " + dest.getName() + ", " + source.getName() + ", cl";
}

//std::string IntelInstructionSet::shr(std::string constant, const Register& result) const {
//}

std::string IntelInstructionSet::add(const Register& operand, const Register& result) const {
    return "add " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::add(const MemoryOperand& operand, const Register& result) const {
    return "add " + result.getName() + ", " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::adc(const Register& operand, const Register& result) const {
    return "adc " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::sub(const Register& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::sub(const MemoryOperand& operand, const Register& result) const {
    return "sub " + result.getName() + ", " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::sbb(const Register& operand, const Register& result) const {
    return "sbb " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::imul(const Register& operand) const {
    return "imul " + operand.getName();
}

std::string IntelInstructionSet::imul(const MemoryOperand& operand) const {
    return "imul qword " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::idiv(const Register& operand) const {
    return "idiv " + operand.getName();
}

std::string IntelInstructionSet::idiv(const MemoryOperand& operand) const {
    return "idiv qword " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::div(const Register& operand) const {
    return "div " + operand.getName();
}

std::string IntelInstructionSet::div(const MemoryOperand& operand) const {
    return "div qword " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::cqo() const {
    return "cqo";
}

std::string IntelInstructionSet::inc(const Register& operand) const {
    return "inc " + operand.getName();
}

std::string IntelInstructionSet::inc(const MemoryOperand& operand) const {
    return "inc qword " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::dec(const Register& operand) const {
    return "dec " + operand.getName();
}

std::string IntelInstructionSet::dec(const MemoryOperand& operand) const {
    return "dec qword " + memoryReference(operand, *this);
}

std::string IntelInstructionSet::neg(const Register& operand) const {
    return "neg " + operand.getName();
}

std::vector<std::string> IntelInstructionSet::bswap(const Register& operand, int widthBytes) const {
    if (widthBytes == 2) {
        return { "rol " + lowWordName(operand) + ", 8", "and " + operand.getName() + ", 0xffff" };
    }
    if (widthBytes == 4) {
        return { "bswap " + lowDwordName(operand) };
    }
    return { "bswap " + operand.getName() };
}

std::string IntelInstructionSet::movqGprToXmm(const Register& gpr, int xmmIndex) const {
    return "movq xmm" + std::to_string(xmmIndex) + ", " + gpr.getName();
}

std::string IntelInstructionSet::movqXmmToGpr(int xmmIndex, const Register& gpr) const {
    return "movq " + gpr.getName() + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::movdGprToXmm(const Register& gpr, int xmmIndex) const {
    return "movd xmm" + std::to_string(xmmIndex) + ", " + lowDwordName(gpr);
}

std::string IntelInstructionSet::movdXmmToGpr(int xmmIndex, const Register& gpr) const {
    return "movd " + lowDwordName(gpr) + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::movDword(const MemoryOperand& source, const Register& dest) const {
    return "mov " + lowDwordName(dest) + ", dword " + memoryReference(source, *this);
}

std::string IntelInstructionSet::movDword(const Register& source, const MemoryOperand& dest) const {
    return "mov dword " + memoryReference(dest, *this) + ", " + lowDwordName(source);
}

std::string IntelInstructionSet::cvtsi2sd(const Register& gpr, int xmmIndex) const {
    return "cvtsi2sd xmm" + std::to_string(xmmIndex) + ", " + gpr.getName();
}

std::string IntelInstructionSet::cvttsd2si(int xmmIndex, const Register& gpr) const {
    return "cvttsd2si " + gpr.getName() + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::cvtsi2ss(const Register& gpr, int xmmIndex) const {
    return "cvtsi2ss xmm" + std::to_string(xmmIndex) + ", " + gpr.getName();
}

std::string IntelInstructionSet::cvttss2si(int xmmIndex, const Register& gpr) const {
    return "cvttss2si " + gpr.getName() + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::cvtss2sd(int srcXmm, int dstXmm) const {
    return "cvtss2sd xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::cvtsd2ss(int srcXmm, int dstXmm) const {
    return "cvtsd2ss xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::addsd(int dstXmm, int srcXmm) const {
    return "addsd xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::subsd(int dstXmm, int srcXmm) const {
    return "subsd xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::mulsd(int dstXmm, int srcXmm) const {
    return "mulsd xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::divsd(int dstXmm, int srcXmm) const {
    return "divsd xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::addss(int dstXmm, int srcXmm) const {
    return "addss xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::subss(int dstXmm, int srcXmm) const {
    return "subss xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::mulss(int dstXmm, int srcXmm) const {
    return "mulss xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::divss(int dstXmm, int srcXmm) const {
    return "divss xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

namespace {

const char* intelX87Width(int sizeBytes) {
    if (sizeBytes == 4) {
        return "dword ";
    }
    if (sizeBytes == 8) {
        return "qword ";
    }
    return "tword ";
}

} // namespace

std::string IntelInstructionSet::loadX87(const MemoryOperand& source, int sizeBytes) const {
    return std::string("fld ") + intelX87Width(sizeBytes) + memoryReference(source, *this);
}

std::string IntelInstructionSet::storeX87(const MemoryOperand& dest, int sizeBytes) const {
    return std::string("fstp ") + intelX87Width(sizeBytes) + memoryReference(dest, *this);
}

std::string IntelInstructionSet::fild(const MemoryOperand& source, int sizeBytes) const {
    return std::string("fild ") + intelX87Width(sizeBytes) + memoryReference(source, *this);
}

std::string IntelInstructionSet::fisttp(const MemoryOperand& dest, int sizeBytes) const {
    return std::string("fisttp ") + intelX87Width(sizeBytes) + memoryReference(dest, *this);
}

std::string IntelInstructionSet::faddp() const {
    return "faddp st1, st0";
}

std::string IntelInstructionSet::fsubp() const {
    return "fsubp st1, st0";
}

std::string IntelInstructionSet::fmulp() const {
    return "fmulp st1, st0";
}

std::string IntelInstructionSet::fdivp() const {
    return "fdivp st1, st0";
}

std::string IntelInstructionSet::fchs() const {
    return "fchs";
}

std::string IntelInstructionSet::fldz() const {
    return "fldz";
}

std::string IntelInstructionSet::fucomip() const {
    return "fucomip st1";
}

std::string IntelInstructionSet::fstpSt0() const {
    return "fstp st0";
}

std::string IntelInstructionSet::loadByteSignExtend(const Register& address, const Register& dest) const {
    return "movsx " + dest.getName() + ", byte [" + address.getName() + "]";
}

std::string IntelInstructionSet::loadByteZeroExtend(const Register& address, const Register& dest) const {
    return "movzx " + dest.getName() + ", byte [" + address.getName() + "]";
}

std::string IntelInstructionSet::loadWordSignExtend(const Register& address, const Register& dest) const {
    return "movsx " + dest.getName() + ", word [" + address.getName() + "]";
}

std::string IntelInstructionSet::loadWordZeroExtend(const Register& address, const Register& dest) const {
    return "movzx " + dest.getName() + ", word [" + address.getName() + "]";
}

std::string IntelInstructionSet::loadDwordSignExtend(const Register& address, const Register& dest) const {
    return "movsxd " + dest.getName() + ", dword [" + address.getName() + "]";
}

std::string IntelInstructionSet::storeByte(const Register& source, const Register& address) const {
    return "mov byte [" + address.getName() + "], " + lowByteName(source);
}

std::string IntelInstructionSet::storeWord(const Register& source, const Register& address) const {
    return "mov word [" + address.getName() + "], " + lowWordName(source);
}

std::string IntelInstructionSet::storeDword(const Register& source, const Register& address) const {
    return "mov dword [" + address.getName() + "], " + lowDwordName(source);
}

} // namespace codegen

