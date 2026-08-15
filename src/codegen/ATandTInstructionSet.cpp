#include "ATandTInstructionSet.h"

#include "Register.h"
#include "RegisterSubreg.h"
#include "MemoryOperand.h"
#include "util/StringLiteralDecode.h"

namespace {

using codegen::Register;

std::string registerAccess(const std::string& name) {
    return "%" + name;
}

std::string registerAccess(const Register& reg) {
    return registerAccess(reg.getName());
}

std::string registerAccess(const Register& reg, int widthBytes) {
    return registerAccess(codegen::gprName(reg, widthBytes));
}

const char* attSuf(int widthBytes) {
    return widthBytes == 4 ? "l" : "q";
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
    return asmSymbol(name) + ":\n\t" + util::toGasByteDirective(escapedValue) + "\n";
}

std::string ATandTInstructionSet::dataObjectLines(const GlobalVariable& global) const {
    if (global.emitAsDword()) {
        const auto values = global.initValuesOrZeros();
        const std::string operand = values.empty() ? "0" : dataOperandText(values.front());
        return asmSymbol(global.name) + ":\n\t.long " + operand + "\n";
    }
    const std::string operands = joinedDataOperands(global);
    return asmSymbol(global.name) + ":\n\t.quad " + (operands.empty() ? "0" : operands) + "\n";
}

std::string ATandTInstructionSet::call(std::string procedureName) const {
    return "call " + asmSymbol(procedureName);
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

std::string ATandTInstructionSet::not_(const Register& reg, int widthBytes) const {
    return std::string("not") + attSuf(widthBytes) + " " + registerAccess(reg, widthBytes);
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
    return "movq " + immediate(constant) + ", " + registerAccess(destination);
}

std::string ATandTInstructionSet::cmp(const Register& leftArgument, const Register& rightArgument,
        int widthBytes) const {
    return std::string("cmp") + attSuf(widthBytes) + " " + registerAccess(rightArgument, widthBytes) + ", "
            + registerAccess(leftArgument, widthBytes);
}

std::string ATandTInstructionSet::cmp(const Register& argument, int constant, int widthBytes) const {
    return std::string("cmp") + attSuf(widthBytes) + " " + constantReference(constant) + ", "
            + registerAccess(argument, widthBytes);
}

std::string ATandTInstructionSet::label(std::string name) const {
    return asmSymbol(name) + ":";
}

std::string ATandTInstructionSet::jmp(std::string label) const {
    return "jmp " + asmSymbol(label);
}

std::string ATandTInstructionSet::je(std::string label) const {
    return "je " + asmSymbol(label);
}

std::string ATandTInstructionSet::jne(std::string label) const {
    return "jne " + asmSymbol(label);
}

std::string ATandTInstructionSet::jg(std::string label) const {
    return "jg " + asmSymbol(label);
}

std::string ATandTInstructionSet::jl(std::string label) const {
    return "jl " + asmSymbol(label);
}

std::string ATandTInstructionSet::jge(std::string label) const {
    return "jge " + asmSymbol(label);
}

std::string ATandTInstructionSet::jle(std::string label) const {
    return "jle " + asmSymbol(label);
}

std::string ATandTInstructionSet::ja(std::string label) const {
    return "ja " + asmSymbol(label);
}

std::string ATandTInstructionSet::jb(std::string label) const {
    return "jb " + asmSymbol(label);
}

std::string ATandTInstructionSet::jae(std::string label) const {
    return "jae " + asmSymbol(label);
}

std::string ATandTInstructionSet::jbe(std::string label) const {
    return "jbe " + asmSymbol(label);
}

std::string ATandTInstructionSet::leave() const {
    return "leave";
}

std::string ATandTInstructionSet::ret() const {
    return "ret";
}

std::string ATandTInstructionSet::xor_(const Register& operand, const Register& result, int widthBytes) const {
    return std::string("xor") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes) + ", "
            + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::or_(const Register& operand, const Register& result, int widthBytes) const {
    return std::string("or") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes) + ", "
            + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::and_(const Register& operand, const Register& result, int widthBytes) const {
    return std::string("and") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes) + ", "
            + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::shl(const Register& result, int widthBytes) const {
    return std::string("shl") + attSuf(widthBytes) + " %cl, " + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::shr(const Register& result, int widthBytes) const {
    return std::string("sar") + attSuf(widthBytes) + " %cl, " + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::lshr(const Register& result, int widthBytes) const {
    return std::string("shr") + attSuf(widthBytes) + " %cl, " + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::shld(const Register& source, const Register& dest) const {
    return "shldq %cl, " + registerAccess(source) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::shrd(const Register& source, const Register& dest) const {
    return "shrdq %cl, " + registerAccess(source) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::add(const Register& operand, const Register& result, int widthBytes) const {
    return std::string("add") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes) + ", "
            + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::adc(const Register& operand, const Register& result) const {
    return "adcq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::sub(const Register& operand, const Register& result, int widthBytes) const {
    return std::string("sub") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes) + ", "
            + registerAccess(result, widthBytes);
}

std::string ATandTInstructionSet::sbb(const Register& operand, const Register& result) const {
    return "sbbq " + registerAccess(operand) + ", " + registerAccess(result);
}

std::string ATandTInstructionSet::imul(const Register& operand, int widthBytes) const {
    return std::string("imul") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::string ATandTInstructionSet::idiv(const Register& operand, int widthBytes) const {
    return std::string("idiv") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::string ATandTInstructionSet::div(const Register& operand, int widthBytes) const {
    return std::string("div") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::string ATandTInstructionSet::cdq() const {
    return "cltd";
}

std::string ATandTInstructionSet::cqo() const {
    return "cqto";
}

std::string ATandTInstructionSet::inc(const Register& operand, int widthBytes) const {
    return std::string("inc") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::string ATandTInstructionSet::dec(const Register& operand, int widthBytes) const {
    return std::string("dec") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::string ATandTInstructionSet::neg(const Register& operand, int widthBytes) const {
    return std::string("neg") + attSuf(widthBytes) + " " + registerAccess(operand, widthBytes);
}

std::vector<std::string> ATandTInstructionSet::bswap(const Register& operand, int widthBytes) const {
    if (widthBytes == 2) {
        return { "rolw $8, %" + lowWordName(operand), "andq $0xffff, %" + operand.getName() };
    }
    if (widthBytes == 4) {
        return { "bswap %" + lowDwordName(operand) };
    }
    return { "bswap %" + operand.getName() };
}

std::string ATandTInstructionSet::ctz(const Register& operand, int widthBytes) const {
    if (widthBytes == 4) {
        return "bsfl %" + lowDwordName(operand) + ", %" + lowDwordName(operand);
    }
    return "bsfq %" + operand.getName() + ", %" + operand.getName();
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
    return "movl " + memoryReference(source, *this) + ", %" + lowDwordName(dest);
}

std::string ATandTInstructionSet::movDword(const Register& source, const MemoryOperand& dest) const {
    return "movl %" + lowDwordName(source) + ", " + memoryReference(dest, *this);
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

std::string ATandTInstructionSet::fchs() const {
    return "fchs";
}

std::string ATandTInstructionSet::fldz() const {
    return "fldz";
}

std::string ATandTInstructionSet::fucomip() const {
    return "fucomip %st(1), %st";
}

std::string ATandTInstructionSet::fstpSt0() const {
    return "fstp %st(0)";
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

std::string ATandTInstructionSet::storeWord(const Register& source, const Register& address) const {
    return "movw %" + lowWordName(source) + ", (%" + address.getName() + ")";
}

std::string ATandTInstructionSet::loadByteSignExtend(const MemoryOperand& source, const Register& dest) const {
    return "movsbq " + memoryReference(source, *this) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadByteZeroExtend(const MemoryOperand& source, const Register& dest) const {
    return "movzbq " + memoryReference(source, *this) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadWordSignExtend(const MemoryOperand& source, const Register& dest) const {
    return "movswq " + memoryReference(source, *this) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadWordZeroExtend(const MemoryOperand& source, const Register& dest) const {
    return "movzwq " + memoryReference(source, *this) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::loadDwordSignExtend(const MemoryOperand& source, const Register& dest) const {
    return "movslq " + memoryReference(source, *this) + ", " + registerAccess(dest);
}

std::string ATandTInstructionSet::storeByte(const Register& source, const MemoryOperand& dest) const {
    return "movb %" + lowByteName(source) + ", " + memoryReference(dest, *this);
}

std::string ATandTInstructionSet::storeWord(const Register& source, const MemoryOperand& dest) const {
    return "movw %" + lowWordName(source) + ", " + memoryReference(dest, *this);
}

} // namespace codegen
