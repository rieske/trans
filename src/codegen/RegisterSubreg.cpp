#include "RegisterSubreg.h"

#include "Register.h"

#include <cctype>

namespace codegen {

std::string lowByteName(const Register& reg) {
    const std::string n = reg.getName();
    if (n == "rax") return "al";
    if (n == "rbx") return "bl";
    if (n == "rcx") return "cl";
    if (n == "rdx") return "dl";
    if (n == "rsi") return "sil";
    if (n == "rdi") return "dil";
    if (n == "rbp") return "bpl";
    if (n == "rsp") return "spl";
    if (n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]))) {
        return n + "b";
    }
    return n;
}

std::string lowWordName(const Register& reg) {
    const std::string n = reg.getName();
    if (n == "rax") return "ax";
    if (n == "rbx") return "bx";
    if (n == "rcx") return "cx";
    if (n == "rdx") return "dx";
    if (n == "rsi") return "si";
    if (n == "rdi") return "di";
    if (n == "rbp") return "bp";
    if (n == "rsp") return "sp";
    if (n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]))) {
        return n + "w";
    }
    return n;
}

std::string lowDwordName(const Register& reg) {
    const std::string n = reg.getName();
    if (n == "rax") return "eax";
    if (n == "rbx") return "ebx";
    if (n == "rcx") return "ecx";
    if (n == "rdx") return "edx";
    if (n == "rsi") return "esi";
    if (n == "rdi") return "edi";
    if (n == "rbp") return "ebp";
    if (n == "rsp") return "esp";
    if (n.size() >= 2 && n[0] == 'r' && std::isdigit(static_cast<unsigned char>(n[1]))) {
        return n + "d";
    }
    return n;
}

} // namespace codegen
