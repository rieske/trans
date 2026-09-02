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
// Two eightbytes. Classifier post-merge cap; type-aware sret uses sysv::classify.
constexpr int REGISTER_RETURN_MAX_BYTES = 2 * MACHINE_WORD_SIZE;

// Callee-local name for the hidden memory-return pointer (first integer arg).
constexpr const char* SRET_SYMBOL_NAME = "__sret";

// Words occupied by a live Value / stack slot. At least 1 so empty-ish slots
// still get a home (matches historical StackMachine / ValueScope wordSlots).
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

// Smallest multiple of alignBytes that is >= bytes. alignBytes <= 1 is a no-op.
inline int alignUp(int bytes, int alignBytes) {
    if (alignBytes <= 1 || bytes <= 0) {
        return bytes;
    }
    const int rem = bytes % alignBytes;
    return rem == 0 ? bytes : bytes + (alignBytes - rem);
}

// Word index whose byte offset is a multiple of alignBytes.
inline int alignWordIndex(int index, int alignBytes) {
    return alignUp(index * MACHINE_WORD_SIZE, alignBytes) / MACHINE_WORD_SIZE;
}

// Take `words` slots from a word cursor so the home is alignBytes-aligned.
inline int takeAlignedWords(int& nextIndex, int alignBytes, int words) {
    const int index = alignWordIndex(nextIndex, alignBytes);
    nextIndex = index + words;
    return index;
}

// homeBytes is the spill-home region. subBytes is that plus the call pad (`sub rsp`).
struct FrameLayout {
    int homeBytes { 0 };
    int subBytes { 0 };
};

inline FrameLayout frameLayout(int localWordCount, int calleeSavedBytes) {
    FrameLayout frame;
    frame.homeBytes = alignUp(localWordCount * MACHINE_WORD_SIZE, STACK_ALIGNMENT);
    frame.subBytes = alignUp(calleeSavedBytes + frame.homeBytes, STACK_ALIGNMENT)
            - calleeSavedBytes;
    return frame;
}

// Word index containing byteOffset (truncating division).
inline int wordIndexAt(int byteOffset) {
    return byteOffset / MACHINE_WORD_SIZE;
}

} // namespace object_abi
} // namespace type

#endif // TYPES_OBJECTABI_H_
