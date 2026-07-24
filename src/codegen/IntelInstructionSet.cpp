#include "IntelInstructionSet.h"

#include "types/ObjectAbi.h"
#include "Register.h"

#include <iostream>
#include <set>
#include <sstream>
#include "util/StringLiteralDecode.h"

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
    std::stringstream preamble;
    preamble << "default rel\n";
    // Always keep libc I/O available for the test harness; also emit every
    // function that is called (or addressed) but not defined in this unit.
    std::set<std::string> externs {
        "scanf", "printf", "malloc", "free", "realloc",
        // Thread-local va_list save-area helpers (runtime/va_tls.c); called from
        // variadic prologues/epilogues as raw assembly so they are not always in quads.
        "__trans_va_set_areas", "__trans_va_pop_areas",
        "__trans_va_get_reg_save", "__trans_va_get_overflow"
    };
    for (const auto& name : externalFunctions) {
        externs.insert(name);
    }
    // Pure-extern variables (from headers) must not get storage; declare them.
    for (const auto& global : globalVariables) {
        if (global.isExternal) {
            externs.insert(global.name);
        }
    }
    for (const auto& name : externs) {
        // extern takes the bare symbol; $ is only for definitions/references.
        preamble << "extern " << name << "\n";
    }
    // SysV AMD64 natural alignment for object types is at most 8 here
    // (type::MAX_ALIGN). String constants are emitted as `db` of arbitrary
    // length; without padding, the next symbol can be misaligned (e.g. long at
    // ...043). glibc pthread_mutex_t / pthread_cond_t require align 8; git
    // grep aborts in futex when grep_mutex is misaligned.
    preamble << "\nsection .data\n";
    for (const auto& constant : constants) {
        preamble << "\talign 8\n";
        preamble << "\t" << nasmSymbol(constant.first) << " " << toConstantDeclaration(constant.second) << "\n";
    }
    // Scalar globals are one qword; multi-word (struct/array) use sizeInBytes qwords.
    // String array globals use db like other constants.
    // global/extern keep the bare name; definitions use $name so reserved words work.
    // static file-scope objects are defined but not exported; extern has no storage here.
    for (const auto& global : globalVariables) {
        if (global.isExternal) {
            continue;
        }
        if (!global.isStatic) {
            preamble << "\tglobal " << global.name << "\n";
        }
        // Align every defined global so odd-length prior `db` cannot break
        // pointer/long/struct alignment required by the ABI and by futex.
        preamble << "\talign 8\n";
        if (global.stringInitializer) {
            preamble << "\t" << nasmSymbol(global.name) << " " << toConstantDeclaration(*global.stringInitializer) << "\n";
        } else if (global.multiWordInitializer && !global.multiWordInitializer->empty()) {
            // Brace-initialized multi-word object: one dq operand per word.
            preamble << "\t" << nasmSymbol(global.name) << " dq ";
            for (std::size_t i = 0; i < global.multiWordInitializer->size(); ++i) {
                if (i > 0) {
                    preamble << ", ";
                }
                preamble << (*global.multiWordInitializer)[i];
            }
            preamble << "\n";
        } else {
            // Multi-word objects (structs, arrays) need full storage. A single dq
            // would let high-offset stores clobber following .data symbols or .text
            // (git: static struct repository the_repo was emitted as 8 bytes).
            // valueWords: at least one home word (matches stack/global Value policy).
            int words = valueWords(global.sizeInBytes);
            preamble << "\t" << nasmSymbol(global.name) << " dq " << global.initializerLiteral;
            for (int i = 1; i < words; ++i) {
                preamble << ", 0";
            }
            preamble << "\n";
        }
    }
    preamble << "\nsection .text\n";
    // definedFunctions entries that are static (TU-local) are listed with a leading
    // '.' sentinel so we do not emit `global` for them (see AssemblyGenerator).
    for (const auto& name : definedFunctions) {
        if (!name.empty() && name[0] == '.') {
            continue;
        }
        preamble << "\tglobal " << name << "\n";
    }
    if (definedFunctions.empty()) {
        preamble << "\tglobal main\n";
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

} // namespace codegen

