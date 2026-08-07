#include "RegisterSubreg.h"

#include "Register.h"

#include <cctype>

namespace codegen {

namespace {

// Legacy names (rax..) map to fixed subregs; r8-r15 use suffix b/w/d.
struct SubregRow {
    const char* full;
    const char* byteName;
    const char* wordName;
    const char* dwordName;
};

constexpr SubregRow kLegacy[] = {
        { "rax", "al", "ax", "eax" },
        { "rbx", "bl", "bx", "ebx" },
        { "rcx", "cl", "cx", "ecx" },
        { "rdx", "dl", "dx", "edx" },
        { "rsi", "sil", "si", "esi" },
        { "rdi", "dil", "di", "edi" },
        { "rbp", "bpl", "bp", "ebp" },
        { "rsp", "spl", "sp", "esp" },
};

const SubregRow* findLegacy(const std::string& n) {
    for (const auto& row : kLegacy) {
        if (n == row.full) {
            return &row;
        }
    }
    return nullptr;
}

bool isNumberedR(const std::string& n) {
    return n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]));
}

enum class Width { Byte, Word, Dword };

std::string subreg(const Register& reg, Width w) {
    const std::string n = reg.getName();
    if (const SubregRow* row = findLegacy(n)) {
        switch (w) {
        case Width::Byte: return row->byteName;
        case Width::Word: return row->wordName;
        case Width::Dword: return row->dwordName;
        }
    }
    if (isNumberedR(n)) {
        switch (w) {
        case Width::Byte: return n + "b";
        case Width::Word: return n + "w";
        case Width::Dword: return n + "d";
        }
    }
    return n;
}

} // namespace

std::string lowByteName(const Register& reg) {
    return subreg(reg, Width::Byte);
}

std::string lowWordName(const Register& reg) {
    return subreg(reg, Width::Word);
}

std::string lowDwordName(const Register& reg) {
    return subreg(reg, Width::Dword);
}

} // namespace codegen
