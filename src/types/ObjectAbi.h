#ifndef TYPES_OBJECTABI_H_
#define TYPES_OBJECTABI_H_

// System V AMD64 word-size helpers for stack/homes/.data packing.
// Size-only (no Type.h): safe for low-level codegen TUs.
// Type-aware sret helpers live in ObjectAbiType.h.

namespace type {
namespace object_abi {

constexpr int MACHINE_WORD_SIZE = 8;
// SysV requires 16-byte stack alignment at call sites.
constexpr int STACK_ALIGNMENT = 2 * MACHINE_WORD_SIZE;
// Size threshold used by sret policy (larger than two integer registers).
// Type-aware gate is typeNeedsMemoryReturn in ObjectAbiType.h (complete records only).
constexpr int REGISTER_RETURN_MAX_BYTES = 2 * MACHINE_WORD_SIZE;

// Callee-local name for the hidden memory-return pointer (first integer arg).
constexpr const char* SRET_SYMBOL_NAME = "__sret";

// Words occupied by a live Value / stack slot. At least 1 so empty-ish slots
// still get a home (matches historical StackMachine / ValueScope wordSlots).
// Current consumers: StackMachine, ValueScope (via valueWords).
inline int valueWords(int sizeInBytes) {
    if (sizeInBytes <= 0) {
        return 1;
    }
    return (sizeInBytes + MACHINE_WORD_SIZE - 1) / MACHINE_WORD_SIZE;
}

// Words in a .data multi-word blob. Zero when size is non-positive (no storage).
inline int dataWords(int sizeInBytes) {
    if (sizeInBytes <= 0) {
        return 0;
    }
    return (sizeInBytes + MACHINE_WORD_SIZE - 1) / MACHINE_WORD_SIZE;
}

// Byte offset of word index (word 0 at the lowest address).
inline int wordByteOffset(int wordIndex) {
    return wordIndex * MACHINE_WORD_SIZE;
}

// Word index containing byteOffset (truncating division).
inline int wordIndexAt(int byteOffset) {
    return byteOffset / MACHINE_WORD_SIZE;
}

// True when an object of this size cannot fit in RAX+RDX (size > 16).
inline bool needsMemoryReturn(int sizeInBytes) {
    return sizeInBytes > REGISTER_RETURN_MAX_BYTES;
}

} // namespace object_abi
} // namespace type

#endif // TYPES_OBJECTABI_H_
