#ifndef UTIL_FLOATINGBITS_H_
#define UTIL_FLOATINGBITS_H_

namespace util {

// Host IEEE image: 32/64-bit in `bits`, 80-bit payload in `bits` + `bitsHi`.
struct FloatingBits {
    unsigned long long bits { 0 };
    unsigned long long bitsHi { 0 };
    int sizeBytes { 8 };
};

} // namespace util

#endif // UTIL_FLOATINGBITS_H_
