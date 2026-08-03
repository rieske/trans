#include "IntelInstructionSet.h"
#include "PreamblePlan.h"

#include "types/ObjectAbi.h"
#include "Register.h"
#include "RegisterSubreg.h"
#include "util/StringLiteralDecode.h"

#include <sstream>
#include <stdexcept>

using type::object_abi::valueWords;

namespace {

// Prefix identifiers with $ so NASM never treats a C symbol as a reserved word
// or instruction mnemonic (e.g. strict, prefetch). The $ form still defines
// the bare symbol name for the linker. Already-escaped names and register
// names (used for indirect call) are left alone.
bool isRegisterName(const std::string& name) {
    static const char* regs[] = {
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "rsp",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
            "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
            "al", "bl", "cl", "dl", "sil", "dil", "bpl", "spl",
            nullptr
    };
    for (const char** p = regs; *p; ++p) {
        if (name == *p) {
            return true;
        }
    }
    return false;
}

std::string nasmSymbol(const std::string& name) {
    if (name.empty() || name[0] == '$' || isRegisterName(name)) {
        return name;
    }
    return "$" + name;
}

std::string memoryOffsetMnemonic(const codegen::Register& memoryBase, int memoryOffset) {
    return "[" + memoryBase.getName() + (memoryOffset ? " + " + std::to_string(memoryOffset) : "") + "]";
}

std::string memoryReference(const codegen::MemoryOperand& operand) {
    if (operand.isGlobal()) {
        return "[rel " + nasmSymbol(operand.label()) + "]";
    }
    return memoryOffsetMnemonic(operand.baseRegister(), operand.offset());
}

[[noreturn]] void badAccessSize(const char* op, int sizeBytes) {
    throw std::runtime_error {
            std::string(op) + ": unsupported size " + std::to_string(sizeBytes) };
}

} // namespace

namespace codegen {

IntelInstructionSet::~IntelInstructionSet() = default;

std::string toConstantDeclaration(std::string escapedConstant) {
    return util::toNasmDbDirective(escapedConstant);
}

std::string IntelInstructionSet::preamble(const std::map<std::string, std::string>& constants,
        const std::vector<GlobalVariable>& globalVariables,
        const std::vector<std::string>& externalFunctions,
        const std::vector<std::string>& definedFunctions) const {
    const PreamblePlan plan = buildPreamblePlan(constants, globalVariables, externalFunctions, definedFunctions);
    std::stringstream preamble;
    preamble << "default rel\n";
    for (const auto& name : plan.externs) {
        preamble << "extern " << name << "\n";
    }
    preamble << "\nsection .data\n";
    for (const auto& item : plan.data) {
        if (item.exportGlobal) {
            preamble << "\tglobal " << item.name << "\n";
        }
        preamble << "\talign 8\n";
        if (item.stringToken) {
            preamble << "\t" << nasmSymbol(item.name) << " " << util::toNasmDbDirective(*item.stringToken) << "\n";
        } else if (item.widthBytes == 4) {
            preamble << "\t" << nasmSymbol(item.name) << " dd "
                    << (item.dataOperands.empty() ? "0" : item.dataOperands.front()) << "\n";
        } else {
            preamble << "\t" << nasmSymbol(item.name) << " dq ";
            for (std::size_t i = 0; i < item.dataOperands.size(); ++i) {
                if (i > 0) {
                    preamble << ", ";
                }
                preamble << item.dataOperands[i];
            }
            preamble << "\n";
        }
    }
    preamble << "\nsection .text\n";
    for (const auto& name : plan.textGlobals) {
        preamble << "\tglobal " << name << "\n";
    }
    preamble << "\n";
    return preamble.str();
}

std::string IntelInstructionSet::label(std::string name) const {
    return nasmSymbol(name) + ":";
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
    return "call " + nasmSymbol(procedureName);
}

std::string IntelInstructionSet::callPlt(std::string procedureName) const {
    return "call " + nasmSymbol(procedureName) + " wrt ..plt";
}

std::string IntelInstructionSet::callIndirect(const Register& target) const {
    return "call " + target.getName();
}

std::string IntelInstructionSet::loadGot(std::string symbolName, const Register& target) const {
    return "mov " + target.getName() + ", [rel " + nasmSymbol(symbolName) + " wrt ..got]";
}

std::string IntelInstructionSet::jmp(std::string label) const {
    return "jmp " + nasmSymbol(label);
}

std::string IntelInstructionSet::je(std::string label) const {
    return "je " + nasmSymbol(label);
}

std::string IntelInstructionSet::jne(std::string label) const {
    return "jne " + nasmSymbol(label);
}

std::string IntelInstructionSet::jg(std::string label) const {
    return "jg " + nasmSymbol(label);
}

std::string IntelInstructionSet::jl(std::string label) const {
    return "jl " + nasmSymbol(label);
}

std::string IntelInstructionSet::jge(std::string label) const {
    return "jge " + nasmSymbol(label);
}

std::string IntelInstructionSet::jle(std::string label) const {
    return "jle " + nasmSymbol(label);
}

std::string IntelInstructionSet::ja(std::string label) const {
    return "ja " + nasmSymbol(label);
}

std::string IntelInstructionSet::jb(std::string label) const {
    return "jb " + nasmSymbol(label);
}

std::string IntelInstructionSet::jae(std::string label) const {
    return "jae " + nasmSymbol(label);
}

std::string IntelInstructionSet::jbe(std::string label) const {
    return "jbe " + nasmSymbol(label);
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


std::string IntelInstructionSet::shr(const Register& result) const {
    // Unsigned >>: zero-fill. Do not use SAR here — UINTMAX_MAX >> n must shrink
    // (git parse-options u16 upper_bound), not stay all-ones.
    return "shr " + result.getName() + ", cl";
}

std::string IntelInstructionSet::sar(const Register& result) const {
    // Signed >>: sign-extend so e.g. (~7)>>2 is -2, not a large positive.
    return "sar " + result.getName() + ", cl";
}


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

std::string IntelInstructionSet::div(const Register& operand) const {
    return "div " + operand.getName();
}

std::string IntelInstructionSet::div(const MemoryOperand& operand) const {
    return "div qword " + memoryReference(operand);
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

std::string IntelInstructionSet::load(const MemoryOperand& source, const Register& dest, int sizeBytes,
        bool isSigned) const {
    const std::string mem = memoryReference(source);
    const std::string d = dest.getName();
    if (sizeBytes == 8) {
        return "mov " + d + ", " + mem;
    }
    if (sizeBytes == 4) {
        return isSigned ? ("movsxd " + d + ", dword " + mem)
                        : ("mov " + lowDwordName(dest) + ", dword " + mem);
    }
    if (sizeBytes == 2) {
        return (isSigned ? "movsx " : "movzx ") + d + ", word " + mem;
    }
    if (sizeBytes == 1) {
        return (isSigned ? "movsx " : "movzx ") + d + ", byte " + mem;
    }
    badAccessSize("load", sizeBytes);
}

std::string IntelInstructionSet::store(const Register& source, const MemoryOperand& dest, int sizeBytes) const {
    const std::string mem = memoryReference(dest);
    if (sizeBytes == 8) {
        return "mov " + mem + ", " + source.getName();
    }
    if (sizeBytes == 4) {
        return "mov dword " + mem + ", " + lowDwordName(source);
    }
    if (sizeBytes == 2) {
        return "mov word " + mem + ", " + lowWordName(source);
    }
    if (sizeBytes == 1) {
        return "mov byte " + mem + ", " + lowByteName(source);
    }
    badAccessSize("store", sizeBytes);
}

std::string IntelInstructionSet::extend(const Register& reg, int sizeBytes, bool isSigned) const {
    const std::string n = reg.getName();
    if (sizeBytes == 1) {
        return (isSigned ? "movsx " : "movzx ") + n + ", " + lowByteName(reg);
    }
    if (sizeBytes == 2) {
        return (isSigned ? "movsx " : "movzx ") + n + ", " + lowWordName(reg);
    }
    if (sizeBytes == 4) {
        return isSigned ? ("movsxd " + n + ", " + lowDwordName(reg))
                        : ("mov " + lowDwordName(reg) + ", " + lowDwordName(reg));
    }
    badAccessSize("extend", sizeBytes);
}

std::string IntelInstructionSet::storeImm(const MemoryOperand& dest, long long imm, int sizeBytes) const {
    const std::string mem = memoryReference(dest);
    if (sizeBytes == 8) {
        return "mov qword " + mem + ", " + std::to_string(imm);
    }
    if (sizeBytes == 4) {
        return "mov dword " + mem + ", " + std::to_string(imm);
    }
    if (sizeBytes == 2) {
        return "mov word " + mem + ", " + std::to_string(imm);
    }
    if (sizeBytes == 1) {
        return "mov byte " + mem + ", " + std::to_string(imm);
    }
    badAccessSize("storeImm", sizeBytes);
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
std::string IntelInstructionSet::cqo() const { return "cqo"; }
std::string IntelInstructionSet::bswap(const Register& reg, int sizeBytes) const {
    if (sizeBytes == 2) {
        return "rol " + lowWordName(reg) + ", 8";
    }
    if (sizeBytes == 4) {
        return "bswap " + lowDwordName(reg);
    }
    return "bswap " + reg.getName();
}
std::string IntelInstructionSet::bsf(const Register& reg) const {
    return "bsf " + reg.getName() + ", " + reg.getName();
}
std::string IntelInstructionSet::shrImm(const Register& reg, int amount) const {
    return "shr " + reg.getName() + ", " + std::to_string(amount);
}

} // namespace codegen

