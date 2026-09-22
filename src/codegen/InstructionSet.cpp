#include "InstructionSet.h"
#include "codegen/InternalError.h"
#include "Register.h"
#include "RegisterSubreg.h"

#include "util/ImmediateFormat.h"
#include "util/StringLiteralDecode.h"

#include <sstream>
#include <type_traits>
#include <utility>
#include <variant>

namespace codegen {

InstructionSet::InstructionSet(AsmSyntax syntax) :
        syntax_ { syntax } {
}

bool InstructionSet::att() const {
    return syntax_ == AsmSyntax::Att;
}

std::string InstructionSet::decorateReg(const std::string& name) const {
    return att() ? "%" + name : name;
}

std::string InstructionSet::reg(const Register& r) const {
    return decorateReg(r.getName());
}

std::string InstructionSet::reg(const Register& r, int widthBytes) const {
    return decorateReg(gprName(r, widthBytes));
}

std::string InstructionSet::imm(int value) const {
    const std::string text = std::to_string(value);
    return att() ? "$" + text : text;
}

std::string InstructionSet::immText(const std::string& constant) const {
    if (!att() || (!constant.empty() && constant[0] == '$')) {
        return constant;
    }
    return "$" + constant;
}

std::string InstructionSet::mem(const MemoryOperand& operand) const {
    if (operand.isGlobal()) {
        const std::string symbol = asmSymbol(operand.label());
        return att() ? symbol + "(%rip)" : "[rel " + symbol + "]";
    }
    const std::string base = operand.baseRegister().getName();
    const int offset = operand.offset();
    if (att()) {
        if (offset == 0) {
            return "(%" + base + ")";
        }
        return std::to_string(offset) + "(%" + base + ")";
    }
    return "[" + base + (offset ? " + " + std::to_string(offset) : "") + "]";
}

std::string InstructionSet::sizedMem(const char* intelSize, const MemoryOperand& operand) const {
    if (att()) {
        return mem(operand);
    }
    return std::string(intelSize) + mem(operand);
}

std::string InstructionSet::extendLoad(std::string_view intelOp, std::string_view attOp,
        const char* intelSize, const MemoryOperand& source, const Register& dest) const {
    return ordered(intelOp, attOp, reg(dest), sizedMem(intelSize, source));
}

std::string InstructionSet::storeNarrow(const char* intelSize, std::string_view attOp,
        const std::string& narrowName, const MemoryOperand& dest) const {
    return ordered("mov", attOp, sizedMem(intelSize, dest), decorateReg(narrowName));
}

std::string InstructionSet::mnemonic(const char* op, int widthBytes) const {
    if (!att()) {
        return op;
    }
    return std::string(op) + (widthBytes == 4 ? "l" : "q");
}

std::string InstructionSet::binary(const char* op, const std::string& dest, const std::string& src,
        int widthBytes) const {
    return ordered(mnemonic(op, widthBytes), dest, src);
}

std::string InstructionSet::doubleShift(const char* op, const Register& source, const Register& dest) const {
    if (att()) {
        return std::string(op) + "q %cl, " + reg(source) + ", " + reg(dest);
    }
    return std::string(op) + " " + dest.getName() + ", " + source.getName() + ", cl";
}

std::string InstructionSet::defined(const std::string& name, const std::string& body) const {
    if (att()) {
        return asmSymbol(name) + ":\n\t" + body + "\n";
    }
    return "\t" + asmSymbol(name) + " " + body + "\n";
}

std::string InstructionSet::unary(const char* op, const std::string& operand, int widthBytes) const {
    return mnemonic(op, widthBytes) + " " + operand;
}

std::string InstructionSet::jump(const char* op, std::string label) const {
    return std::string(op) + " " + asmSymbol(label);
}

std::string InstructionSet::call(std::string procedureName) const {
    return "call " + asmSymbol(procedureName);
}

std::string InstructionSet::callPlt(std::string procedureName) const {
    const char* suffix = att() ? "@plt" : " wrt ..plt";
    return "call " + asmSymbol(procedureName) + suffix;
}

std::string InstructionSet::callIndirect(const Register& target) const {
    if (att()) {
        return "call *" + reg(target);
    }
    return "call " + target.getName();
}

std::string InstructionSet::loadGot(std::string symbolName, const Register& target) const {
    const std::string symbol = asmSymbol(symbolName);
    if (att()) {
        return "movq " + symbol + "@GOTPCREL(%rip), " + reg(target);
    }
    return "mov " + target.getName() + ", [rel " + symbol + " wrt ..got]";
}

std::string InstructionSet::label(std::string name) const {
    return asmSymbol(name) + ":";
}

std::string InstructionSet::jmp(std::string target) const { return jump("jmp", std::move(target)); }
std::string InstructionSet::je(std::string target) const { return jump("je", std::move(target)); }
std::string InstructionSet::jne(std::string target) const { return jump("jne", std::move(target)); }
std::string InstructionSet::jg(std::string target) const { return jump("jg", std::move(target)); }
std::string InstructionSet::jl(std::string target) const { return jump("jl", std::move(target)); }
std::string InstructionSet::jge(std::string target) const { return jump("jge", std::move(target)); }
std::string InstructionSet::jle(std::string target) const { return jump("jle", std::move(target)); }
std::string InstructionSet::ja(std::string target) const { return jump("ja", std::move(target)); }
std::string InstructionSet::jb(std::string target) const { return jump("jb", std::move(target)); }
std::string InstructionSet::jae(std::string target) const { return jump("jae", std::move(target)); }
std::string InstructionSet::jbe(std::string target) const { return jump("jbe", std::move(target)); }

std::string InstructionSet::xmm(int index) const {
    return decorateReg("xmm" + std::to_string(index));
}

std::string InstructionSet::ordered(std::string_view op, const std::string& dest, const std::string& src) const {
    return ordered(op, op, dest, src);
}

std::string InstructionSet::ordered(std::string_view intelOp, std::string_view attOp, const std::string& dest,
        const std::string& src) const {
    if (att()) {
        return std::string(attOp) + " " + src + ", " + dest;
    }
    return std::string(intelOp) + " " + dest + ", " + src;
}

namespace {

const char* x87Size(int sizeBytes) {
    if (sizeBytes == 4) {
        return "dword ";
    }
    if (sizeBytes == 8) {
        return "qword ";
    }
    return "tword ";
}

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

} // namespace

std::string InstructionSet::x87Mem(const char* intelOp, const char* attOp, int sizeBytes,
        const MemoryOperand& operand) const {
    if (att()) {
        return std::string(attOp) + mem(operand);
    }
    return std::string(intelOp) + " " + x87Size(sizeBytes) + mem(operand);
}

std::string InstructionSet::movqGprToXmm(const Register& gpr, int xmmIndex) const {
    return ordered("movq", xmm(xmmIndex), reg(gpr));
}

std::string InstructionSet::movqXmmToGpr(int xmmIndex, const Register& gpr) const {
    return ordered("movq", reg(gpr), xmm(xmmIndex));
}

std::string InstructionSet::movdGprToXmm(const Register& gpr, int xmmIndex) const {
    return ordered("movd", xmm(xmmIndex), reg(gpr, 4));
}

std::string InstructionSet::movdXmmToGpr(int xmmIndex, const Register& gpr) const {
    return ordered("movd", reg(gpr, 4), xmm(xmmIndex));
}

std::string InstructionSet::cvtsi2sd(const Register& gpr, int xmmIndex) const {
    return ordered("cvtsi2sd", "cvtsi2sdq", xmm(xmmIndex), reg(gpr));
}

std::string InstructionSet::cvttsd2si(int xmmIndex, const Register& gpr) const {
    return ordered("cvttsd2si", reg(gpr), xmm(xmmIndex));
}

std::string InstructionSet::cvtsi2ss(const Register& gpr, int xmmIndex) const {
    return ordered("cvtsi2ss", "cvtsi2ssq", xmm(xmmIndex), reg(gpr));
}

std::string InstructionSet::cvttss2si(int xmmIndex, const Register& gpr) const {
    return ordered("cvttss2si", reg(gpr), xmm(xmmIndex));
}

std::string InstructionSet::cvtss2sd(int srcXmm, int dstXmm) const {
    return ordered("cvtss2sd", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::cvtsd2ss(int srcXmm, int dstXmm) const {
    return ordered("cvtsd2ss", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::addsd(int dstXmm, int srcXmm) const {
    return ordered("addsd", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::subsd(int dstXmm, int srcXmm) const {
    return ordered("subsd", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::mulsd(int dstXmm, int srcXmm) const {
    return ordered("mulsd", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::divsd(int dstXmm, int srcXmm) const {
    return ordered("divsd", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::addss(int dstXmm, int srcXmm) const {
    return ordered("addss", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::subss(int dstXmm, int srcXmm) const {
    return ordered("subss", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::mulss(int dstXmm, int srcXmm) const {
    return ordered("mulss", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::divss(int dstXmm, int srcXmm) const {
    return ordered("divss", xmm(dstXmm), xmm(srcXmm));
}

std::string InstructionSet::loadX87(const MemoryOperand& source, int sizeBytes) const {
    return x87Mem("fld", attX87Load(sizeBytes), sizeBytes, source);
}

std::string InstructionSet::storeX87(const MemoryOperand& dest, int sizeBytes) const {
    return x87Mem("fstp", attX87Store(sizeBytes), sizeBytes, dest);
}

std::string InstructionSet::fild(const MemoryOperand& source, int sizeBytes) const {
    return x87Mem("fild", sizeBytes == 8 ? "fildll " : "fildl ", sizeBytes, source);
}

std::string InstructionSet::fisttp(const MemoryOperand& dest, int sizeBytes) const {
    return x87Mem("fisttp", sizeBytes == 8 ? "fisttpll " : "fisttpl ", sizeBytes, dest);
}

std::string InstructionSet::faddp() const {
    return att() ? "faddp %st, %st(1)" : "faddp st1, st0";
}

std::string InstructionSet::fsubp() const {
    return att() ? "fsubrp %st, %st(1)" : "fsubp st1, st0";
}

std::string InstructionSet::fmulp() const {
    return att() ? "fmulp %st, %st(1)" : "fmulp st1, st0";
}

std::string InstructionSet::fdivp() const {
    return att() ? "fdivrp %st, %st(1)" : "fdivp st1, st0";
}

std::string InstructionSet::fchs() const {
    return "fchs";
}

std::string InstructionSet::fldz() const {
    return "fldz";
}

std::string InstructionSet::fucomip() const {
    return att() ? "fucomip %st(1), %st" : "fucomip st1";
}

std::string InstructionSet::fstpSt0() const {
    return att() ? "fstp %st(0)" : "fstp st0";
}

std::vector<std::string> InstructionSet::bswap(const Register& operand, int widthBytes) const {
    if (widthBytes == 2) {
        return {
            ordered("rol", "rolw", decorateReg(lowWordName(operand)), imm(8)),
            ordered("and", "andq", reg(operand), immText("0xffff")),
        };
    }
    return { "bswap " + reg(operand, widthBytes) };
}

std::string InstructionSet::ctz(const Register& operand, int widthBytes) const {
    const std::string spelled = reg(operand, widthBytes);
    return binary("bsf", spelled, spelled, widthBytes);
}

std::string InstructionSet::leave() const {
    return "leave";
}

std::string InstructionSet::ret() const {
    return "ret";
}

std::string InstructionSet::push(const Register& r) const {
    return unary("push", reg(r), 8);
}

std::string InstructionSet::pop(const Register& r) const {
    return unary("pop", reg(r), 8);
}

std::string InstructionSet::lea(const MemoryOperand& source, const Register& target) const {
    return binary("lea", reg(target), mem(source), 8);
}

std::string InstructionSet::mov(const Register& from, const MemoryOperand& destination) const {
    return binary("mov", mem(destination), reg(from), 8);
}

std::string InstructionSet::mov(const Register& from, const Register& to) const {
    if (&from == &to) {
        return "";
    }
    return binary("mov", reg(to), reg(from), 8);
}

std::string InstructionSet::mov(const MemoryOperand& source, const Register& to) const {
    return binary("mov", reg(to), mem(source), 8);
}

std::string InstructionSet::mov(std::string constant, const MemoryOperand& destination) const {
    return binary("mov", sizedMem("qword ", destination), immText(constant), 8);
}

std::string InstructionSet::mov(std::string constant, const Register& to) const {
    return binary("mov", reg(to), immText(constant), 8);
}

std::string InstructionSet::movDword(const MemoryOperand& source, const Register& dest) const {
    return binary("mov", reg(dest, 4), sizedMem("dword ", source), 4);
}

std::string InstructionSet::movDword(const Register& source, const MemoryOperand& dest) const {
    return binary("mov", sizedMem("dword ", dest), reg(source, 4), 4);
}

std::string InstructionSet::loadByteSignExtend(const MemoryOperand& source, const Register& dest) const {
    return extendLoad("movsx", "movsbq", "byte ", source, dest);
}

std::string InstructionSet::loadByteZeroExtend(const MemoryOperand& source, const Register& dest) const {
    return extendLoad("movzx", "movzbq", "byte ", source, dest);
}

std::string InstructionSet::loadWordSignExtend(const MemoryOperand& source, const Register& dest) const {
    return extendLoad("movsx", "movswq", "word ", source, dest);
}

std::string InstructionSet::loadWordZeroExtend(const MemoryOperand& source, const Register& dest) const {
    return extendLoad("movzx", "movzwq", "word ", source, dest);
}

std::string InstructionSet::loadDwordSignExtend(const MemoryOperand& source, const Register& dest) const {
    return extendLoad("movsxd", "movslq", "dword ", source, dest);
}

std::string InstructionSet::storeByte(const Register& source, const MemoryOperand& dest) const {
    return storeNarrow("byte ", "movb", lowByteName(source), dest);
}

std::string InstructionSet::storeWord(const Register& source, const MemoryOperand& dest) const {
    return storeNarrow("word ", "movw", lowWordName(source), dest);
}

std::string InstructionSet::add(const Register& r, int constant) const {
    return binary("add", reg(r), imm(constant), 8);
}

std::string InstructionSet::sub(const Register& r, int constant) const {
    return binary("sub", reg(r), imm(constant), 8);
}

std::string InstructionSet::not_(const Register& r, int widthBytes) const {
    return unary("not", reg(r, widthBytes), widthBytes);
}

std::string InstructionSet::cmp(const Register& leftArgument, const Register& rightArgument,
        int widthBytes) const {
    return binary("cmp", reg(leftArgument, widthBytes), reg(rightArgument, widthBytes), widthBytes);
}

std::string InstructionSet::cmp(const Register& argument, int constant, int widthBytes) const {
    return binary("cmp", reg(argument, widthBytes), imm(constant), widthBytes);
}

std::string InstructionSet::xor_(const Register& operand, const Register& result, int widthBytes) const {
    return binary("xor", reg(result, widthBytes), reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::or_(const Register& operand, const Register& result, int widthBytes) const {
    return binary("or", reg(result, widthBytes), reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::and_(const Register& operand, const Register& result, int widthBytes) const {
    return binary("and", reg(result, widthBytes), reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::shl(const Register& result, int widthBytes) const {
    return binary("shl", reg(result, widthBytes), decorateReg("cl"), widthBytes);
}

std::string InstructionSet::shr(const Register& result, int widthBytes) const {
    return binary("sar", reg(result, widthBytes), decorateReg("cl"), widthBytes);
}

std::string InstructionSet::lshr(const Register& result, int widthBytes) const {
    return binary("shr", reg(result, widthBytes), decorateReg("cl"), widthBytes);
}

std::string InstructionSet::shld(const Register& source, const Register& dest) const {
    return doubleShift("shld", source, dest);
}

std::string InstructionSet::shrd(const Register& source, const Register& dest) const {
    return doubleShift("shrd", source, dest);
}

std::string InstructionSet::add(const Register& operand, const Register& result, int widthBytes) const {
    return binary("add", reg(result, widthBytes), reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::adc(const Register& operand, const Register& result) const {
    return binary("adc", reg(result), reg(operand), 8);
}

std::string InstructionSet::sub(const Register& operand, const Register& result, int widthBytes) const {
    return binary("sub", reg(result, widthBytes), reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::sbb(const Register& operand, const Register& result) const {
    return binary("sbb", reg(result), reg(operand), 8);
}

std::string InstructionSet::imul(const Register& operand, int widthBytes) const {
    return unary("imul", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::idiv(const Register& operand, int widthBytes) const {
    return unary("idiv", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::div(const Register& operand, int widthBytes) const {
    return unary("div", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::cdq() const {
    return att() ? "cltd" : "cdq";
}

std::string InstructionSet::cqo() const {
    return att() ? "cqto" : "cqo";
}

std::string InstructionSet::inc(const Register& operand, int widthBytes) const {
    return unary("inc", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::dec(const Register& operand, int widthBytes) const {
    return unary("dec", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::neg(const Register& operand, int widthBytes) const {
    return unary("neg", reg(operand, widthBytes), widthBytes);
}

std::string InstructionSet::preamblePrefix() const {
    return att() ? std::string {} : "default rel\n";
}

std::string InstructionSet::asmSymbol(const std::string& name) const {
    // $foo is the symbol foo; $ does not change the ELF name.
    if (att() || name.empty() || name[0] == '$') {
        return name;
    }
    return "$" + name;
}

std::string InstructionSet::globl(const std::string& name) const {
    return (att() ? ".globl " : "global ") + name;
}

std::string InstructionSet::externDirective(const std::string& name) const {
    return (att() ? ".extern " : "extern ") + name;
}

std::string InstructionSet::globlDataLine(const std::string& name) const {
    const std::string line = globl(name) + "\n";
    return att() ? line : "\t" + line;
}

std::string InstructionSet::dataSectionHeader() const {
    return att() ? "\n.section .data\n" : "\nsection .data\n";
}

std::string InstructionSet::textSectionHeader() const {
    return att() ? "\n.section .text\n\n" : "\nsection .text\n\n";
}

std::string InstructionSet::constantLine(const std::string& name, const std::string& escapedValue) const {
    return defined(name, att() ? util::toGasByteDirective(escapedValue) : util::toNasmDbDirective(escapedValue));
}

std::string InstructionSet::alignDirective(int bytes) const {
    return att() ? "\t.align " + std::to_string(bytes) + "\n" : "\talign " + std::to_string(bytes) + "\n";
}

std::string InstructionSet::dataObjectLines(const GlobalVariable& global) const {
    if (global.emitAsDword()) {
        const auto values = global.initValuesOrZeros();
        const std::string operand = values.empty() ? "0" : dataOperandText(values.front());
        return defined(global.name, (att() ? ".long " : "dd ") + operand);
    }
    const std::string operands = joinedDataOperands(global);
    return defined(global.name, (att() ? ".quad " : "dq ") + (operands.empty() ? "0" : operands));
}

std::string InstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalSymbols) const {
    std::stringstream out;
    out << preamblePrefix();
    for (const auto& name : externalSymbols) {
        out << externDirective(name) << "\n";
    }
    out << dataSectionHeader();
    for (const auto& constant : constants) {
        out << constantLine(constant.first, constant.second);
    }
    for (const auto& global : globalVariables) {
        if (global.emission == ObjectEmission::Reference) {
            continue;
        }
        if (global.emission == ObjectEmission::DefineExternal) {
            out << globlDataLine(global.name);
        }
        if (global.alignBytes > 1) {
            out << alignDirective(global.alignBytes);
        }
        out << dataObjectLines(global);
    }
    out << textSectionHeader();
    return out.str();
}

std::string InstructionSet::dataOperandText(const symbols::StaticInitValue& value) const {
    return std::visit([this](const auto& arm) -> std::string {
        using T = std::decay_t<decltype(arm)>;
        if constexpr (std::is_same_v<T, symbols::StaticWord>) {
            return util::wordImmediate(arm.bits);
        } else if constexpr (std::is_same_v<T, symbols::StaticAddress>) {
            const std::string symbol = asmSymbol(arm.symbol);
            if (arm.addend == 0) {
                return symbol;
            }
            if (arm.addend > 0) {
                return symbol + "+" + std::to_string(arm.addend);
            }
            return symbol + std::to_string(arm.addend);
        } else {
            internalError("dataOperandText expects a storage word or address");
        }
    }, value);
}

std::string InstructionSet::joinedDataOperands(const GlobalVariable& global) const {
    const auto values = global.initValuesOrZeros();
    std::stringstream out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            out << ", ";
        }
        out << dataOperandText(values[i]);
    }
    return out.str();
}

} // namespace codegen
