#include "ATandTInstructionSet.h"

#include "InstructionSet.h"
#include "Register.h"
#include "RegisterSubreg.h"
#include "MemoryOperand.h"
#include "util/StringLiteralDecode.h"

#include <sstream>
#include <stdexcept>

namespace {

using codegen::Register;
using codegen::lowByteName;
using codegen::lowWordName;
using codegen::lowDwordName;

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

std::string memoryReference(const codegen::MemoryOperand& operand, const codegen::InstructionSet& isa) {
    if (operand.isGlobal()) {
        return isa.asmSymbol(operand.label()) + "(%rip)";
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
    return "\t.balign 8\n" + asmSymbol(name) + ":\n\t" + util::toGasByteDirective(escapedValue) + "\n";
}

std::string ATandTInstructionSet::dataObjectLines(const GlobalVariable& global) const {
    const auto operands = formattedDataOperands(global);
    if (global.emitAsDword()) {
        return "\t.balign 8\n" + asmSymbol(global.name) + ":\n\t.long "
                + (operands.empty() ? "0" : operands.front()) + "\n";
    }
    std::stringstream out;
    out << "\t.balign 8\n" << asmSymbol(global.name) << ":\n\t.quad ";
    for (std::size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << operands[i];
    }
    out << "\n";
    return out.str();
}


std::string ATandTInstructionSet::callPlt(std::string procedureName) const {
    return "call " + asmSymbol(procedureName) + "@plt";
}

std::string ATandTInstructionSet::callIndirect(const Register& target) const {
    return "call *" + registerAccess(target);
}

std::string ATandTInstructionSet::loadGot(std::string symbolName, const Register& target) const {
    return "movq " + asmSymbol(symbolName) + "@GOTPCREL(%rip), " + registerAccess(target);
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
    return "leaq " + memoryReference(source, *this) + ", " + registerAccess(target);
}

std::string ATandTInstructionSet::not_(const Register& reg) const {
    return "notq " + registerAccess(reg);
}

std::string ATandTInstructionSet::mov(const Register& source, const MemoryOperand& destination) const {
    return "movq " + registerAccess(source) + ", " + memoryReference(destination, *this);
}

std::string ATandTInstructionSet::mov(const Register& source, const Register& destination) const {
    if (&source == &destination) {
        return "";
    }
    return "movq " + registerAccess(source) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(const MemoryOperand& source, const Register& destination) const {
    return "movq " + memoryReference(source, *this) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::mov(std::string constant, const MemoryOperand& destination) const {
    return "movq " + immediate(constant) + ", " + memoryReference(destination, *this);
}

std::string ATandTInstructionSet::mov(std::string constant, const Register& destination) const {
    // Numeric/hex immediates only; labels use lea via assignLabelAddress.
    return "movq " + immediate(constant) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const MemoryOperand& rightArgument) const {
    return "cmpq " + memoryReference(rightArgument, *this) + ", " + registerAccess(leftArgument);
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument) const {
    return "cmpq " + registerAccess(rightArgument) + ", " + registerAccess(leftArgument);
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, const Register& rightArgument) const {
    return "cmpq " + registerAccess(rightArgument) + ", " + memoryReference(leftArgument, *this);
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant) const {
    return "cmpq " + constantReference(constant) + ", " + registerAccess(argument);
}

std::string ATandTInstructionSet::cmp(const MemoryOperand& leftArgument, int constant) const {
    return "cmpq " + constantReference(constant) + ", " + memoryReference(leftArgument, *this);
}















std::string ATandTInstructionSet::xor_(const Register& operand, const Register& result) const {
    return "xorq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::xor_(const MemoryOperand& operand, const Register& result) const {
    return "xorq " + memoryReference(operand, *this) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result) const {
    return "orq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::or_(const MemoryOperand& operand, const Register& result) const {
    return "orq " + memoryReference(operand, *this) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result) const {
    return "andq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::and_(const MemoryOperand& operand, const Register& result) const {
    return "andq " + memoryReference(operand, *this) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::shl(const Register& result) const {
    return "shlq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::shr(const Register& result) const {
    return "shrq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::sar(const Register& result) const {
    return "sarq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::shld(const Register& source, const Register& dest) const {
    return "shldq %cl, " + registerAccess(source) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::shrd(const Register& source, const Register& dest) const {
    return "shrdq %cl, " + registerAccess(source) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::add(const Register& operand, const Register& result) const {
    return "addq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::add(const MemoryOperand& operand, const Register& result) const {
    return "addq " + memoryReference(operand, *this) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::adc(const Register& operand, const Register& result) const {
    return "adcq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result) const {
    return "subq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const MemoryOperand& operand, const Register& result) const {
    return "subq " + memoryReference(operand, *this) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sbb(const Register& operand, const Register& result) const {
    return "sbbq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::imul(const Register& operand) const {
    return "imulq " + registerAccess(operand);
}

std::string ATandTInstructionSet::imul(const MemoryOperand& operand) const {
    return "imulq " + memoryReference(operand, *this);
}

std::string ATandTInstructionSet::idiv(const Register& operand) const {
    return "idivq " + registerAccess(operand);
}

std::string ATandTInstructionSet::idiv(const MemoryOperand& operand) const {
    return "idivq " + memoryReference(operand, *this);
}

std::string ATandTInstructionSet::div(const Register& operand) const {
    return "divq " + registerAccess(operand);
}

std::string ATandTInstructionSet::div(const MemoryOperand& operand) const {
    return "divq " + memoryReference(operand, *this);
}

std::string ATandTInstructionSet::inc(const Register& operand) const {
    return "incq " + registerAccess(operand);
}

std::string ATandTInstructionSet::inc(const MemoryOperand& operand) const {
    return "incq " + memoryReference(operand, *this);
}

std::string ATandTInstructionSet::dec(const Register& operand) const {
    return "decq " + registerAccess(operand);
}

std::string ATandTInstructionSet::dec(const MemoryOperand& operand) const {
    return "decq " + memoryReference(operand, *this);
}

std::string ATandTInstructionSet::neg(const Register& operand) const {
    return "negq " + registerAccess(operand);
}

std::vector<std::string> ATandTInstructionSet::bswapW(const Register& operand, AccessWidth width) const {
    if (width == AccessWidth::B2) {
        return { "rolw $8, %" + lowWordName(operand), "andq $0xffff, %" + operand.getName() };
    }
    if (width == AccessWidth::B4) {
        return { "bswap %" + lowDwordName(operand) };
    }
    return { "bswap %" + operand.getName() };
}

std::string ATandTInstructionSet::loadW(const MemoryOperand& source, const Register& dest,
        AccessWidth width, bool isSigned) const {
    const std::string mem = memoryReference(source, *this);
    if (width == AccessWidth::B8) {
        return "movq " + mem + ", %" + dest.getName();
    }
    if (width == AccessWidth::B4) {
        return isSigned ? ("movslq " + mem + ", %" + dest.getName())
                        : ("movl " + mem + ", %" + lowDwordName(dest));
    }
    if (width == AccessWidth::B2) {
        return (isSigned ? "movswq " : "movzwq ") + mem + ", %" + dest.getName();
    }
    return (isSigned ? "movsbq " : "movzbq ") + mem + ", %" + dest.getName();
}

std::string ATandTInstructionSet::storeW(const Register& source, const MemoryOperand& dest,
        AccessWidth width) const {
    const std::string mem = memoryReference(dest, *this);
    if (width == AccessWidth::B8) {
        return "movq %" + source.getName() + ", " + mem;
    }
    if (width == AccessWidth::B4) {
        return "movl %" + lowDwordName(source) + ", " + mem;
    }
    if (width == AccessWidth::B2) {
        return "movw %" + lowWordName(source) + ", " + mem;
    }
    return "movb %" + lowByteName(source) + ", " + mem;
}

std::string ATandTInstructionSet::extendW(const Register& reg, AccessWidth width, bool isSigned) const {
    if (width == AccessWidth::B4) {
        return isSigned ? ("movslq %" + lowDwordName(reg) + ", %" + reg.getName())
                        : ("movl %" + lowDwordName(reg) + ", %" + lowDwordName(reg));
    }
    if (width == AccessWidth::B2) {
        return (isSigned ? "movswq %" : "movzwq %") + lowWordName(reg) + ", %" + reg.getName();
    }
    return (isSigned ? "movsbq %" : "movzbq %") + lowByteName(reg) + ", %" + reg.getName();
}

std::string ATandTInstructionSet::storeImmW(const MemoryOperand& dest, long long imm, AccessWidth width) const {
    const char* prefix = width == AccessWidth::B8 ? "movq $"
            : width == AccessWidth::B4 ? "movl $"
            : width == AccessWidth::B2 ? "movw $" : "movb $";
    return std::string(prefix) + std::to_string(imm) + ", " + memoryReference(dest, *this);
}

std::string ATandTInstructionSet::sseGprXmm(SseGprXmmDir dir, SseWidth width, const Register& gpr,
        int xmmIndex) const {
    const std::string x = "%xmm" + std::to_string(xmmIndex);
    if (width == SseWidth::F32) {
        const std::string d = "%" + lowDwordName(gpr);
        return dir == SseGprXmmDir::GprToXmm ? ("movd " + d + ", " + x) : ("movd " + x + ", " + d);
    }
    const std::string q = "%" + gpr.getName();
    return dir == SseGprXmmDir::GprToXmm ? ("movq " + q + ", " + x) : ("movq " + x + ", " + q);
}

std::string ATandTInstructionSet::sseXmmToMem(int xmmIndex, const MemoryOperand& dest) const {
    return "movq %xmm" + std::to_string(xmmIndex) + ", " + memoryReference(dest, *this);
}

std::string ATandTInstructionSet::sseCvtIntToXmm(const Register& gpr, int xmmIndex, SseWidth dest) const {
    const char* op = dest == SseWidth::F32 ? "cvtsi2ssq %" : "cvtsi2sdq %";
    return std::string(op) + gpr.getName() + ", %xmm" + std::to_string(xmmIndex);
}

std::string ATandTInstructionSet::sseCvtTruncToGpr(int xmmIndex, const Register& gpr, SseWidth src) const {
    const char* op = src == SseWidth::F32 ? "cvttss2si %xmm" : "cvttsd2si %xmm";
    return std::string(op) + std::to_string(xmmIndex) + ", %" + gpr.getName();
}

std::string ATandTInstructionSet::sseCvtFloat(SseWidth from, SseWidth to, int srcXmm, int dstXmm) const {
    const char* op = sseCvtFloatWidens(from, to) ? "cvtss2sd %xmm" : "cvtsd2ss %xmm";
    return std::string(op) + std::to_string(srcXmm) + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::sseBin(SseBin op, SseWidth width, int dstXmm, int srcXmm) const {
    return std::string(sseBinMnemonic(op, width)) + " %xmm" + std::to_string(srcXmm)
            + ", %xmm" + std::to_string(dstXmm);
}

std::string ATandTInstructionSet::sseUcomi(SseWidth width, int leftXmm, int rightXmm) const {
    const char* op = width == SseWidth::F32 ? "ucomiss " : "ucomisd ";
    return std::string(op) + "%xmm" + std::to_string(rightXmm) + ", %xmm" + std::to_string(leftXmm);
}
std::string ATandTInstructionSet::cqo() const { return "cqto"; }
std::string ATandTInstructionSet::bsf(const Register& reg) const {
    return "bsfq %" + reg.getName() + ", %" + reg.getName();
}

namespace {

const char* attX87Load(int sizeBytes) {
    if (sizeBytes == 4) {
        return "flds ";
    }
    if (sizeBytes == 8) {
        return "fldl ";
    }
    return "fldt ";
}

const char* attX87Store(int sizeBytes) {
    if (sizeBytes == 4) {
        return "fstps ";
    }
    if (sizeBytes == 8) {
        return "fstpl ";
    }
    return "fstpt ";
}

const char* attFild(int sizeBytes) {
    return sizeBytes == 8 ? "fildll " : "fildl ";
}

const char* attFisttp(int sizeBytes) {
    return sizeBytes == 8 ? "fisttpll " : "fisttpl ";
}

} // namespace

std::string ATandTInstructionSet::loadX87(const MemoryOperand& source, int sizeBytes) const {
    return std::string(attX87Load(sizeBytes)) + memoryReference(source, *this);
}

std::string ATandTInstructionSet::storeX87(const MemoryOperand& dest, int sizeBytes) const {
    return std::string(attX87Store(sizeBytes)) + memoryReference(dest, *this);
}

std::string ATandTInstructionSet::fild(const MemoryOperand& source, int sizeBytes) const {
    return std::string(attFild(sizeBytes)) + memoryReference(source, *this);
}

std::string ATandTInstructionSet::fisttp(const MemoryOperand& dest, int sizeBytes) const {
    return std::string(attFisttp(sizeBytes)) + memoryReference(dest, *this);
}

std::string ATandTInstructionSet::faddp() const {
    return "faddp %st, %st(1)";
}

std::string ATandTInstructionSet::fsubp() const {
    return "fsubrp %st, %st(1)";
}

std::string ATandTInstructionSet::fmulp() const {
    return "fmulp %st, %st(1)";
}

std::string ATandTInstructionSet::fdivp() const {
    return "fdivrp %st, %st(1)";
}



std::string ATandTInstructionSet::fucomip() const {
    return "fucomip %st(1), %st";
}

std::string ATandTInstructionSet::fstpSt0() const {
    return "fstp %st(0)";
}

std::string ATandTInstructionSet::shrImm(const Register& reg, int amount) const {
    return "shrq $" + std::to_string(amount) + ", %" + reg.getName();
}

} // namespace codegen
