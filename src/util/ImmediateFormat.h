#ifndef UTIL_IMMEDIATEFORMAT_H_
#define UTIL_IMMEDIATEFORMAT_H_

#include "IntegerLiteral.h"

#include <sstream>
#include <string>

namespace util {

// Hex assembler immediate (float IEEE bits; integers outside signed 32-bit).
inline std::string hexImmediate(unsigned long long bits) {
    std::ostringstream hex;
    hex << "0x" << std::hex << bits;
    return hex.str();
}

// NASM/GAS qword operand: hex when a signed 32-bit imm would not fit.
inline std::string wordImmediate(unsigned long long v) {
    if (v > 0x7fffffffull) {
        return hexImmediate(v);
    }
    return std::to_string(v);
}

// C integer lexeme (suffixes ok). False when the value does not fit in 64 bits.
inline bool integerLiteralImmediate(const std::string& token, std::string& out) {
    IntegerLiteral lit;
    if (!parseIntegerLiteral(token, lit) || lit.value > ~0ull) {
        return false;
    }
    out = wordImmediate(static_cast<unsigned long long>(lit.value));
    return true;
}

} // namespace util

#endif // UTIL_IMMEDIATEFORMAT_H_
