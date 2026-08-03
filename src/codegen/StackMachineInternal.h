#ifndef STACKMACHINE_INTERNAL_H_
#define STACKMACHINE_INTERNAL_H_

// Shared helpers for StackMachine translation units. Not part of the public API.

#include <vector>

#include "types/ObjectAbiType.h"
#include "Register.h"

namespace codegen {
namespace stack_machine_detail {

constexpr int MACHINE_WORD_SIZE = type::object_abi::MACHINE_WORD_SIZE;
constexpr int STACK_ALIGNMENT = type::object_abi::STACK_ALIGNMENT;

// Clamp C access sizes to a typed load/store width (1/2/4/8).
inline int accessWidth(int sizeBytes) {
    if (sizeBytes == 1 || sizeBytes == 2 || sizeBytes == 4) {
        return sizeBytes;
    }
    return MACHINE_WORD_SIZE;
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

using stack_machine_detail::MACHINE_WORD_SIZE;
using stack_machine_detail::STACK_ALIGNMENT;
using stack_machine_detail::accessWidth;
using stack_machine_detail::registerInList;

} // namespace codegen

#endif // STACKMACHINE_INTERNAL_H_
