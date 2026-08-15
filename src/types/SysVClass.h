#ifndef TYPES_SYSVCLASS_H_
#define TYPES_SYSVCLASS_H_

#include <array>

namespace type {
namespace sysv {

enum class Class {
    NoClass,
    Integer,
    Sse,
    SseUp,
    X87,
    X87Up,
    ComplexX87,
    Memory
};

inline bool isInteger(Class k) {
    return k == Class::Integer;
}

inline bool isSse(Class k) {
    return k == Class::Sse || k == Class::SseUp;
}

inline bool isX87(Class k) {
    return k == Class::X87 || k == Class::X87Up || k == Class::ComplexX87;
}

// Narrow INTEGER scalars in a GPR: caller sign/zero-extends; aggregates stay None.
enum class GprExtend {
    None,
    Sign,
    Zero
};

// After merge: MEMORY (count 0), empty, or 1-2 eightbytes.
struct Classification {
    bool memory { false };
    int count { 0 };
    int alignBytes { 8 };
    GprExtend gprExtend { GprExtend::None };
    std::array<Class, 2> eightbytes { Class::NoClass, Class::NoClass };

    bool inRegisters() const {
        if (memory || count <= 0) {
            return false;
        }
        for (int i = 0; i < count; ++i) {
            const Class k = eightbytes[static_cast<std::size_t>(i)];
            if (!isInteger(k) && !isSse(k)) {
                return false;
            }
        }
        return true;
    }

    bool hasX87() const {
        for (int i = 0; i < count; ++i) {
            if (isX87(eightbytes[static_cast<std::size_t>(i)])) {
                return true;
            }
        }
        return false;
    }
};

inline int integerEightbytes(const Classification& c) {
    int n = 0;
    for (int i = 0; i < c.count; ++i) {
        if (isInteger(c.eightbytes[static_cast<std::size_t>(i)])) {
            ++n;
        }
    }
    return n;
}

inline int sseEightbytes(const Classification& c) {
    int n = 0;
    for (int i = 0; i < c.count; ++i) {
        if (isSse(c.eightbytes[static_cast<std::size_t>(i)])) {
            ++n;
        }
    }
    return n;
}

inline Classification scalar(Class k, int sizeBytes = 8) {
    Classification c;
    if (sizeBytes <= 0) {
        return c;
    }
    c.alignBytes = sizeBytes > 8 ? 8 : sizeBytes;
    if (sizeBytes > 8) {
        c.memory = true;
        return c;
    }
    c.count = 1;
    c.eightbytes[0] = k;
    return c;
}

inline Classification integerScalar(int sizeBytes = 8) {
    Classification c = scalar(Class::Integer, sizeBytes);
    if (sizeBytes > 0 && sizeBytes < 8) {
        c.gprExtend = GprExtend::Zero;
    }
    return c;
}

inline Classification sseScalar(int sizeBytes = 8) {
    return scalar(Class::Sse, sizeBytes);
}

// Values with no C type: only the three ISO complex sizes are valid.
inline Classification complexClass(int sizeBytes) {
    Classification c;
    if (sizeBytes == 8) {
        c.count = 1;
        c.eightbytes[0] = Class::Sse;
        c.alignBytes = 4;
        return c;
    }
    if (sizeBytes == 16) {
        c.count = 2;
        c.eightbytes[0] = Class::Sse;
        c.eightbytes[1] = Class::Sse;
        c.alignBytes = 8;
        return c;
    }
    if (sizeBytes == 32) {
        c.count = 1;
        c.eightbytes[0] = Class::ComplexX87;
        c.alignBytes = 16;
        return c;
    }
    return c;
}

inline bool isComplexX87(const Classification& c) {
    return !c.memory && c.count > 0 && c.eightbytes[0] == Class::ComplexX87;
}

inline Classification memoryClass() {
    Classification c;
    c.memory = true;
    return c;
}

} // namespace sysv
} // namespace type

#endif
