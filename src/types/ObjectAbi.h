#ifndef TYPES_OBJECTABI_H_
#define TYPES_OBJECTABI_H_

// System V AMD64 object sizing and aggregate return policy shared by
// StackMachine (live Values), CodeGeneratingVisitor (call/return quads),
// and semantic global multi-word .data flattening.
// Lives under types/ so semantic_analyzer does not depend on codegen/.

namespace codegen {
namespace object_abi {

constexpr int MACHINE_WORD_SIZE = 8;
// SysV requires 16-byte stack alignment at call sites.
constexpr int STACK_ALIGNMENT = 2 * MACHINE_WORD_SIZE;
// Aggregates larger than two integer registers return via hidden pointer (sret).
constexpr int REGISTER_RETURN_MAX_BYTES = 2 * MACHINE_WORD_SIZE;
constexpr int REGISTER_RETURN_MAX_WORDS = 2;

// Callee-local name for the hidden memory-return pointer (first integer arg).
constexpr const char* SRET_SYMBOL_NAME = "__sret";

// Words occupied by a live Value / stack slot. At least 1 so empty-ish slots
// still get a home (matches historical StackMachine::wordCount).
inline int valueWords(int sizeInBytes) {
    int words = (sizeInBytes + MACHINE_WORD_SIZE - 1) / MACHINE_WORD_SIZE;
    return words < 1 ? 1 : words;
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

// System V: only aggregates (struct/union/array) use memory return; scalars never sret.
// Prefer aggregateNeedsMemoryReturn(t.isAggregate(), t.getSize()) at call sites.
inline bool aggregateNeedsMemoryReturn(bool isAggregate, int sizeInBytes) {
    return isAggregate && needsMemoryReturn(sizeInBytes);
}

// True when valueWords exceeds register-return capacity (words > 2).
inline bool valueNeedsMemoryReturn(int sizeInBytes) {
    return valueWords(sizeInBytes) > REGISTER_RETURN_MAX_WORDS;
}

} // namespace object_abi
} // namespace codegen

#endif // TYPES_OBJECTABI_H_
