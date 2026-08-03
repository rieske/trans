#include "ATandTInstructionSet.h"
#include "PreamblePlan.h"

#include "Register.h"
#include "RegisterSubreg.h"
#include "MemoryOperand.h"
#include "util/StringLiteralDecode.h"

#include <sstream>
#include <stdexcept>

namespace {

using codegen::Register;
using codegen::bareSymbol;
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

std::string memoryReference(const codegen::MemoryOperand& operand) {
    if (operand.isGlobal()) {
        return bareSymbol(operand.label()) + "(%rip)";
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

// Match Intel toNasmDbDirective: exact decoded bytes including trailing NUL.
std::string toGasByteDirective(const std::string& token) {
    const auto bytes = util::decodeStringLiteralBytes(token);
    std::ostringstream declaration;
    declaration << ".byte ";
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        if (i > 0) {
            declaration << ", ";
        }
        declaration << static_cast<unsigned>(bytes[i]);
    }
    return declaration.str();
}

[[noreturn]] void badAccessSize(const char* op, int sizeBytes) {
    throw std::runtime_error {
            std::string(op) + ": unsupported size " + std::to_string(sizeBytes) };
}

} // namespace

namespace codegen {

ATandTInstructionSet::~ATandTInstructionSet() = default;

std::string ATandTInstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) const {
    const PreamblePlan plan = buildPreamblePlan(constants, globalVariables, externalFunctions, definedFunctions);
    std::stringstream preamble;
    for (const auto& name : plan.externs) {
        preamble << ".extern " << name << "\n";
    }
    preamble << "\n.section .data\n";
    for (const auto& item : plan.data) {
        if (item.exportGlobal) {
            preamble << ".globl " << item.name << "\n";
        }
        preamble << "\t.balign 8\n";
        if (item.stringToken) {
            preamble << item.name << ":\n\t" << toGasByteDirective(*item.stringToken) << "\n";
        } else if (item.widthBytes == 4) {
            preamble << item.name << ":\n\t.long "
                    << (item.dataOperands.empty() ? "0" : item.dataOperands.front()) << "\n";
        } else {
            preamble << item.name << ":\n\t.quad ";
            for (std::size_t i = 0; i < item.dataOperands.size(); ++i) {
                if (i > 0) {
                    preamble << ", ";
                }
                preamble << item.dataOperands[i];
            }
            preamble << "\n";
        }
    }
    preamble << "\n.section .text\n";
    for (const auto& name : plan.textGlobals) {
        preamble << ".globl " << name << "\n";
    }
    preamble << "\n";
    return preamble.str();
}

std::string ATandTInstructionSet::call(std::string procedureName) const {
    return "call " + bareSymbol(std::move(procedureName));
}

std::string ATandTInstructionSet::callPlt(std::string procedureName) const {
    return "call " + bareSymbol(std::move(procedureName)) + "@plt";
}

std::string ATandTInstructionSet::callIndirect(const Register& target) const {
    return "call *" + registerAccess(target);
}

std::string ATandTInstructionSet::loadGot(std::string symbolName, const Register& target) const {
    return "movq " + bareSymbol(std::move(symbolName)) + "@GOTPCREL(%rip), " + registerAccess(target);
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
    // Numeric/hex immediates only; labels use lea via assignLabelAddress.
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
    return bareSymbol(std::move(name)) + ":";
}

std::string ATandTInstructionSet::jmp(std::string label) const {
    return "jmp " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::je(std::string label) const {
    return "je " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jne(std::string label) const {
    return "jne " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jg(std::string label) const {
    return "jg " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jl(std::string label) const {
    return "jl " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jge(std::string label) const {
    return "jge " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jle(std::string label) const {
    return "jle " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::ja(std::string label) const {
    return "ja " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jb(std::string label) const {
    return "jb " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jae(std::string label) const {
    return "jae " + bareSymbol(std::move(label));
}

std::string ATandTInstructionSet::jbe(std::string label) const {
    return "jbe " + bareSymbol(std::move(label));
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
    return "shrq %cl, " + registerAccess(result);
}

std::string ATandTInstructionSet::sar(const Register& result) const {
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

std::string ATandTInstructionSet::div(const Register& operand) const {
    return "divq " + registerAccess(operand);
}

std::string ATandTInstructionSet::div(const MemoryOperand& operand) const {
    return "divq " + memoryReference(operand);
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

std::string ATandTInstructionSet::load(const MemoryOperand& source, const Register& dest, int sizeBytes,
        bool isSigned) const {
    const std::string mem = memoryReference(source);
    const std::string d = dest.getName();
    if (sizeBytes == 8) {
        return "movq " + mem + ", %" + d;
    }
    if (sizeBytes == 4) {
        return isSigned ? ("movslq " + mem + ", %" + d)
                        : ("movl " + mem + ", %" + lowDwordName(dest));
    }
    if (sizeBytes == 2) {
        return (isSigned ? "movswq " : "movzwq ") + mem + ", %" + d;
    }
    if (sizeBytes == 1) {
        return (isSigned ? "movsbq " : "movzbq ") + mem + ", %" + d;
    }
    badAccessSize("load", sizeBytes);
}

std::string ATandTInstructionSet::store(const Register& source, const MemoryOperand& dest, int sizeBytes) const {
    const std::string mem = memoryReference(dest);
    if (sizeBytes == 8) {
        return "movq %" + source.getName() + ", " + mem;
    }
    if (sizeBytes == 4) {
        return "movl %" + lowDwordName(source) + ", " + mem;
    }
    if (sizeBytes == 2) {
        return "movw %" + lowWordName(source) + ", " + mem;
    }
    if (sizeBytes == 1) {
        return "movb %" + lowByteName(source) + ", " + mem;
    }
    badAccessSize("store", sizeBytes);
}

std::string ATandTInstructionSet::extend(const Register& reg, int sizeBytes, bool isSigned) const {
    const std::string n = reg.getName();
    if (sizeBytes == 1) {
        return (isSigned ? "movsbq %" : "movzbq %") + lowByteName(reg) + ", %" + n;
    }
    if (sizeBytes == 2) {
        return (isSigned ? "movswq %" : "movzwq %") + lowWordName(reg) + ", %" + n;
    }
    if (sizeBytes == 4) {
        return isSigned ? ("movslq %" + lowDwordName(reg) + ", %" + n)
                        : ("movl %" + lowDwordName(reg) + ", %" + lowDwordName(reg));
    }
    badAccessSize("extend", sizeBytes);
}

std::string ATandTInstructionSet::storeImm(const MemoryOperand& dest, long long imm, int sizeBytes) const {
    const std::string mem = memoryReference(dest);
    if (sizeBytes == 8) {
        return "movq $" + std::to_string(imm) + ", " + mem;
    }
    if (sizeBytes == 4) {
        return "movl $" + std::to_string(imm) + ", " + mem;
    }
    if (sizeBytes == 2) {
        return "movw $" + std::to_string(imm) + ", " + mem;
    }
    if (sizeBytes == 1) {
        return "movb $" + std::to_string(imm) + ", " + mem;
    }
    badAccessSize("storeImm", sizeBytes);
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
    return "movq %xmm" + std::to_string(xmmIndex) + ", " + memoryReference(dest);
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
std::string ATandTInstructionSet::cqo() const { return "cqto"; }
std::string ATandTInstructionSet::bswap(const Register& reg, int sizeBytes) const {
    if (sizeBytes == 2) {
        return "rolw $8, %" + lowWordName(reg);
    }
    if (sizeBytes == 4) {
        return "bswap %" + lowDwordName(reg);
    }
    return "bswap %" + reg.getName();
}
std::string ATandTInstructionSet::bsf(const Register& reg) const {
    return "bsfq %" + reg.getName() + ", %" + reg.getName();
}
std::string ATandTInstructionSet::shrImm(const Register& reg, int amount) const {
    return "shrq $" + std::to_string(amount) + ", %" + reg.getName();
}

} // namespace codegen
