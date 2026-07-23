#ifndef STACKMACHINE_INTERNAL_H_
#define STACKMACHINE_INTERNAL_H_

// Shared helpers for StackMachine translation units. Not part of the public API.

#include <string>
#include <vector>

#include "ObjectAbi.h"
#include "Register.h"

namespace codegen {
namespace stack_machine_detail {

constexpr int MACHINE_WORD_SIZE = object_abi::MACHINE_WORD_SIZE;
constexpr int STACK_ALIGNMENT = object_abi::STACK_ALIGNMENT;

// Low 8-bit name for a 64-bit GP register (Intel/NASM).
inline std::string reg8Name(const Register& reg) {
    const std::string& n = reg.getName();
    if (n == "rax") return "al";
    if (n == "rbx") return "bl";
    if (n == "rcx") return "cl";
    if (n == "rdx") return "dl";
    if (n == "rsi") return "sil";
    if (n == "rdi") return "dil";
    if (n == "rbp") return "bpl";
    if (n == "rsp") return "spl";
    // r8-r15 -> r8b-r15b
    if (n.size() >= 2 && n[0] == 'r' && n[1] >= '0' && n[1] <= '9') {
        return n + "b";
    }
    return n;
}

// 16-bit name for a 64-bit GP register (Intel/NASM).
inline std::string reg16Name(const Register& reg) {
    const std::string& n = reg.getName();
    if (n == "rax") return "ax";
    if (n == "rbx") return "bx";
    if (n == "rcx") return "cx";
    if (n == "rdx") return "dx";
    if (n == "rsi") return "si";
    if (n == "rdi") return "di";
    if (n == "rbp") return "bp";
    if (n == "rsp") return "sp";
    // r8-r15 -> r8w-r15w
    if (n.size() >= 2 && n[0] == 'r' && n[1] >= '0' && n[1] <= '9') {
        return n + "w";
    }
    return n;
}

// 32-bit name for a 64-bit GP register (Intel/NASM).
inline std::string reg32Name(const Register& reg) {
    const std::string& n = reg.getName();
    if (n == "rax") return "eax";
    if (n == "rbx") return "ebx";
    if (n == "rcx") return "ecx";
    if (n == "rdx") return "edx";
    if (n == "rsi") return "esi";
    if (n == "rdi") return "edi";
    if (n == "rbp") return "ebp";
    if (n == "rsp") return "esp";
    // r8-r15 -> r8d-r15d
    if (n.size() >= 2 && n[0] == 'r' && n[1] >= '0' && n[1] <= '9') {
        return n + "d";
    }
    return n;
}

inline std::string memoryAt(const Register& base, int offset) {
    if (offset == 0) {
        return "[" + base.getName() + "]";
    }
    if (offset > 0) {
        return "[" + base.getName() + " + " + std::to_string(offset) + "]";
    }
    return "[" + base.getName() + " - " + std::to_string(-offset) + "]";
}

inline bool registerInList(Register* reg, const std::vector<Register*>& list) {
    for (auto* excluded : list) {
        if (reg == excluded) {
            return true;
        }
    }
    return false;
}

} // namespace stack_machine_detail

// Bring helpers into codegen for StackMachine*.cpp bodies (match prior anonymous-namespace use).
using stack_machine_detail::MACHINE_WORD_SIZE;
using stack_machine_detail::STACK_ALIGNMENT;
using stack_machine_detail::reg8Name;
using stack_machine_detail::reg16Name;
using stack_machine_detail::reg32Name;
using stack_machine_detail::memoryAt;
using stack_machine_detail::registerInList;

} // namespace codegen

#endif // STACKMACHINE_INTERNAL_H_
