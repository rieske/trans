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

// After merge: MEMORY (count 0), empty, or 1-2 eightbytes.
struct Classification {
    bool memory { false };
    int count { 0 };
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
    if (sizeBytes > 8) {
        c.memory = true;
        return c;
    }
    c.count = 1;
    c.eightbytes[0] = k;
    return c;
}

inline Classification integerScalar(int sizeBytes = 8) {
    return scalar(Class::Integer, sizeBytes);
}

inline Classification sseScalar(int sizeBytes = 8) {
    return scalar(Class::Sse, sizeBytes);
}

inline Classification memoryClass() {
    Classification c;
    c.memory = true;
    return c;
}

} // namespace sysv
} // namespace type

#endif
