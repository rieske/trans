#include "IntelInstructionSet.h"

#include "Register.h"
#include "RegisterSubreg.h"
#include "util/StringLiteralDecode.h"

#include <sstream>

namespace {

std::string memoryOffsetMnemonic(const codegen::Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

const char* intelMemSize(codegen::AccessWidth width) {
    switch (width) {
    case codegen::AccessWidth::B1:
        return "byte ";
    case codegen::AccessWidth::B2:
        return "word ";
    case codegen::AccessWidth::B4:
        return "dword ";
    case codegen::AccessWidth::B8:
        return "qword ";
    }
    throw std::logic_error("invalid AccessWidth");
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

std::string IntelInstructionSet::memoryReference(const MemoryOperand& operand) const {
    if (operand.isGlobal()) {
        return "[rel " + asmSymbol(operand.label()) + "]";
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

std::string IntelInstructionSet::dataSectionHeader() const {
    return "\nsection .data\n";
}

std::string IntelInstructionSet::textSectionHeader() const {
    return "\nsection .text\n\n";
}

std::string IntelInstructionSet::constantLine(const std::string& name, const std::string& escapedValue) const {
    return "\talign 8\n\t" + asmSymbol(name) + " " + util::toNasmDbDirective(escapedValue) + "\n";
}

std::string IntelInstructionSet::dataObjectLines(const GlobalVariable& global) const {
    const auto operands = formattedDataOperands(global);
    const std::string label = asmSymbol(global.name);
    if (global.emitAsDword()) {
        return "\talign 8\n\t" + label + " dd "
                + (operands.empty() ? "0" : operands.front()) + "\n";
    }
    std::stringstream out;
    out << "\talign 8\n\t" << label << " dq ";
    for (std::size_t i = 0; i < operands.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << operands[i];
    }
    out << "\n";
    return out.str();
}


std::string IntelInstructionSet::push(const Register& reg) const {
    return "push " + reg.getName();
}

std::string IntelInstructionSet::pop(const Register& reg) const {
    return "pop " + reg.getName();
}

std::string IntelInstructionSet::add(const Register& reg, int constant, GprWidth width) const {
    return "add " + gprName(reg, gprWidthBytes(width)) + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::sub(const Register& reg, int constant, GprWidth width) const {
    return "sub " + gprName(reg, gprWidthBytes(width)) + ", " + std::to_string(constant);
}

std::string IntelInstructionSet::lea(const MemoryOperand& source, const Register& target) const {
    return "lea " + target.getName() + ", " + memoryReference(source);
}

std::string IntelInstructionSet::not_(const Register& reg, GprWidth width) const {
    return "not " + gprName(reg, gprWidthBytes(width));
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

std::string IntelInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument,
        GprWidth width) const {
    return "cmp " + gprName(leftArgument, gprWidthBytes(width)) + ", " + gprName(rightArgument, gprWidthBytes(width));
}

std::string IntelInstructionSet::cmp(const Register& argument, int constant, GprWidth width) const {
    return "cmp " + gprName(argument, gprWidthBytes(width)) + ", " + std::to_string(constant);
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















std::string IntelInstructionSet::xor_(const Register& operand, const Register& result, GprWidth width) const {
    return "xor " + gprName(result, gprWidthBytes(width)) + ", " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::or_(const Register& operand, const Register& result, GprWidth width) const {
    return "or " + gprName(result, gprWidthBytes(width)) + ", " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::and_(const Register& operand, const Register& result, GprWidth width) const {
    return "and " + gprName(result, gprWidthBytes(width)) + ", " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::shl(const Register& result, GprWidth width) const {
    return "shl " + gprName(result, gprWidthBytes(width)) + ", cl";
}

std::string IntelInstructionSet::shr(const Register& result, GprWidth width) const {
    return "shr " + gprName(result, gprWidthBytes(width)) + ", cl";
}

std::string IntelInstructionSet::sar(const Register& result, GprWidth width) const {
    return "sar " + gprName(result, gprWidthBytes(width)) + ", cl";
}

std::string IntelInstructionSet::shld(const Register& source, const Register& dest) const {
    return "shld " + dest.getName() + ", " + source.getName() + ", cl";
}

std::string IntelInstructionSet::shrd(const Register& source, const Register& dest) const {
    return "shrd " + dest.getName() + ", " + source.getName() + ", cl";
}

std::string IntelInstructionSet::add(const Register& operand, const Register& result, GprWidth width) const {
    return "add " + gprName(result, gprWidthBytes(width)) + ", " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::adc(const Register& operand, const Register& result) const {
    return "adc " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::sub(const Register& operand, const Register& result, GprWidth width) const {
    return "sub " + gprName(result, gprWidthBytes(width)) + ", " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::sbb(const Register& operand, const Register& result) const {
    return "sbb " + result.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::imul(const Register& operand, GprWidth width) const {
    return "imul " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::idiv(const Register& operand, GprWidth width) const {
    return "idiv " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::div(const Register& operand, GprWidth width) const {
    return "div " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::cdq() const {
    return "cdq";
}

std::string IntelInstructionSet::cqo() const {
    return "cqo";
}

std::string IntelInstructionSet::inc(const Register& operand, GprWidth width) const {
    return "inc " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::incW(const MemoryOperand& operand, AccessWidth width) const {
    return "inc " + std::string(intelMemSize(width)) + memoryReference(operand);
}

std::string IntelInstructionSet::dec(const Register& operand, GprWidth width) const {
    return "dec " + gprName(operand, gprWidthBytes(width));
}

std::string IntelInstructionSet::decW(const MemoryOperand& operand, AccessWidth width) const {
    return "dec " + std::string(intelMemSize(width)) + memoryReference(operand);
}

std::string IntelInstructionSet::neg(const Register& operand, GprWidth width) const {
    return "neg " + gprName(operand, gprWidthBytes(width));
}

std::vector<std::string> IntelInstructionSet::bswapW(const Register& operand, AccessWidth width) const {
    if (width == AccessWidth::B2) {
        return { "rol " + lowWordName(operand) + ", 8", "and " + operand.getName() + ", 0xffff" };
    }
    if (width == AccessWidth::B4) {
        return { "bswap " + lowDwordName(operand) };
    }
    return { "bswap " + operand.getName() };
}

std::string IntelInstructionSet::ctz(const Register& operand, int widthBytes) const {
    if (widthBytes == 4) {
        return "bsf " + lowDwordName(operand) + ", " + lowDwordName(operand);
    }
    return "bsf " + operand.getName() + ", " + operand.getName();
}

std::string IntelInstructionSet::loadW(const MemoryOperand& source, const Register& dest,
        AccessWidth width, bool isSigned) const {
    const std::string mem = memoryReference(source);
    if (width == AccessWidth::B8) {
        return "mov " + dest.getName() + ", " + mem;
    }
    if (width == AccessWidth::B4) {
        return isSigned ? ("movsxd " + dest.getName() + ", dword " + mem)
                        : ("mov " + lowDwordName(dest) + ", dword " + mem);
    }
    if (width == AccessWidth::B2) {
        return (isSigned ? "movsx " : "movzx ") + dest.getName() + ", word " + mem;
    }
    return (isSigned ? "movsx " : "movzx ") + dest.getName() + ", byte " + mem;
}

std::string IntelInstructionSet::storeW(const Register& source, const MemoryOperand& dest,
        AccessWidth width) const {
    const std::string mem = memoryReference(dest);
    if (width == AccessWidth::B8) {
        return "mov " + mem + ", " + source.getName();
    }
    if (width == AccessWidth::B4) {
        return "mov dword " + mem + ", " + lowDwordName(source);
    }
    if (width == AccessWidth::B2) {
        return "mov word " + mem + ", " + lowWordName(source);
    }
    return "mov byte " + mem + ", " + lowByteName(source);
}

std::string IntelInstructionSet::extendW(const Register& reg, AccessWidth width, bool isSigned) const {
    if (width == AccessWidth::B4) {
        return isSigned ? ("movsxd " + reg.getName() + ", " + lowDwordName(reg))
                        : ("mov " + lowDwordName(reg) + ", " + lowDwordName(reg));
    }
    if (width == AccessWidth::B2) {
        return (isSigned ? "movsx " : "movzx ") + reg.getName() + ", " + lowWordName(reg);
    }
    return (isSigned ? "movsx " : "movzx ") + reg.getName() + ", " + lowByteName(reg);
}

std::string IntelInstructionSet::storeImmW(const MemoryOperand& dest, long long imm, AccessWidth width) const {
    const std::string mem = memoryReference(dest);
    const char* prefix = width == AccessWidth::B8 ? "mov qword "
            : width == AccessWidth::B4 ? "mov dword "
            : width == AccessWidth::B2 ? "mov word " : "mov byte ";
    return std::string(prefix) + mem + ", " + std::to_string(imm);
}

std::string IntelInstructionSet::sseGprXmm(SseGprXmmDir dir, SseWidth width, const Register& gpr,
        int xmmIndex) const {
    const std::string x = "xmm" + std::to_string(xmmIndex);
    if (width == SseWidth::F32) {
        const std::string d = lowDwordName(gpr);
        return dir == SseGprXmmDir::GprToXmm ? ("movd " + x + ", " + d) : ("movd " + d + ", " + x);
    }
    return dir == SseGprXmmDir::GprToXmm
            ? ("movq " + x + ", " + gpr.getName())
            : ("movq " + gpr.getName() + ", " + x);
}

std::string IntelInstructionSet::sseXmmToMem(int xmmIndex, const MemoryOperand& dest) const {
    return "movq " + memoryReference(dest) + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::sseCvtIntToXmm(const Register& gpr, int xmmIndex, SseWidth dest) const {
    const char* op = dest == SseWidth::F32 ? "cvtsi2ss " : "cvtsi2sd ";
    return std::string(op) + "xmm" + std::to_string(xmmIndex) + ", " + gpr.getName();
}

std::string IntelInstructionSet::sseCvtTruncToGpr(int xmmIndex, const Register& gpr, SseWidth src) const {
    const char* op = src == SseWidth::F32 ? "cvttss2si " : "cvttsd2si ";
    return std::string(op) + gpr.getName() + ", xmm" + std::to_string(xmmIndex);
}

std::string IntelInstructionSet::sseCvtFloat(SseWidth from, SseWidth to, int srcXmm, int dstXmm) const {
    const char* op = sseCvtFloatWidens(from, to) ? "cvtss2sd " : "cvtsd2ss ";
    return std::string(op) + "xmm" + std::to_string(dstXmm) + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::sseBin(SseBin op, SseWidth width, int dstXmm, int srcXmm) const {
    return std::string(sseBinMnemonic(op, width)) + " xmm" + std::to_string(dstXmm)
            + ", xmm" + std::to_string(srcXmm);
}

std::string IntelInstructionSet::sseUcomi(SseWidth width, int leftXmm, int rightXmm) const {
    const char* op = width == SseWidth::F32 ? "ucomiss " : "ucomisd ";
    return std::string(op) + "xmm" + std::to_string(leftXmm) + ", xmm" + std::to_string(rightXmm);
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
    return std::string("fld ") + intelX87Width(sizeBytes) + memoryReference(source);
}

std::string IntelInstructionSet::storeX87(const MemoryOperand& dest, int sizeBytes) const {
    return std::string("fstp ") + intelX87Width(sizeBytes) + memoryReference(dest);
}

std::string IntelInstructionSet::fild(const MemoryOperand& source, int sizeBytes) const {
    return std::string("fild ") + intelX87Width(sizeBytes) + memoryReference(source);
}

std::string IntelInstructionSet::fisttp(const MemoryOperand& dest, int sizeBytes) const {
    return std::string("fisttp ") + intelX87Width(sizeBytes) + memoryReference(dest);
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



std::string IntelInstructionSet::fucomip() const {
    return "fucomip st1";
}

std::string IntelInstructionSet::fstpSt0() const {
    return "fstp st0";
}

std::string IntelInstructionSet::shrImm(const Register& reg, int amount) const {
    return "shr " + reg.getName() + ", " + std::to_string(amount);
}

} // namespace codegen

