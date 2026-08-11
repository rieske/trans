#ifndef SYMBOLS_STATIC_INIT_H_
#define SYMBOLS_STATIC_INIT_H_

#include <string>
#include <variant>

namespace symbols {

// Translation-time value for static-duration initializers (C 6.6).
struct StaticInteger {
    long value { 0 };
};

struct StaticFloat {
    unsigned long long bits { 0 };
    int sizeBytes { 8 };
};

struct StaticAddress {
    std::string symbol;
    long addend { 0 };
};

using StaticInitValue = std::variant<StaticInteger, StaticFloat, StaticAddress>;

} // namespace symbols

#endif
