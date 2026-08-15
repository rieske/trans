#ifndef UTIL_INTEGERLITERAL_H_
#define UTIL_INTEGERLITERAL_H_

#include <string>

namespace util {

__extension__ typedef unsigned __int128 WideUInt;

struct IntegerLiteral {
    WideUInt value { 0 };
    int base { 10 };
    bool uns { false };
    bool lng { false };
};

inline int integerDigitValue(char c, int base) {
    int d = -1;
    if (c >= '0' && c <= '9') {
        d = c - '0';
    } else if (c >= 'a' && c <= 'f') {
        d = c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        d = c - 'A' + 10;
    }
    return (d >= 0 && d < base) ? d : -1;
}

// Decode a C integer lexeme (optional 0/0x prefix and U/L suffixes) into 128 bits.
inline bool parseIntegerLiteral(const std::string& token, IntegerLiteral& out) {
    if (token.empty() || token.front() == '\'') {
        return false;
    }
    std::size_t i = 0;
    int base = 10;
    if (token.size() >= 2 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X')) {
        base = 16;
        i = 2;
    } else if (token.size() >= 2 && token[0] == '0'
            && token[1] >= '0' && token[1] <= '7') {
        base = 8;
        i = 1;
    }
    WideUInt v = 0;
    bool any = false;
    const WideUInt max = ~(WideUInt)0;
    for (; i < token.size(); ++i) {
        const int d = integerDigitValue(token[i], base);
        if (d < 0) {
            break;
        }
        if (v > max / static_cast<unsigned>(base)) {
            return false;
        }
        v = v * static_cast<unsigned>(base) + static_cast<unsigned>(d);
        any = true;
    }
    if (!any) {
        return false;
    }
    bool u = false;
    bool l = false;
    for (; i < token.size(); ++i) {
        const char c = token[i];
        if (c == 'u' || c == 'U') {
            u = true;
        } else if (c == 'l' || c == 'L') {
            l = true;
        } else {
            return false;
        }
    }
    out.value = v;
    out.base = base;
    out.uns = u;
    out.lng = l;
    return true;
}

} // namespace util

#endif
