#ifndef SYMBOLS_STATIC_INIT_H_
#define SYMBOLS_STATIC_INIT_H_

#include <string>
#include <variant>
#include <vector>

#include "types/IntegerConstant.h"
#include "types/ObjectAbi.h"

namespace symbols {

// Translation-time value for static-duration initializers (C 6.6).
struct StaticInteger {
    type::IntegerConstant value;

    StaticInteger() = default;
    StaticInteger(long host) :
            value { type::fromHostLong(host) } {
    }
    StaticInteger(type::IntegerConstant ice) :
            value { std::move(ice) } {
    }
};

// Folded floating value. 16-byte x87 uses bits + bitsHi; storage expands via asDataWords.
struct StaticFloat {
    unsigned long long bits { 0 };
    unsigned long long bitsHi { 0 };
    int sizeBytes { 8 };
};

struct StaticAddress {
    std::string symbol;
    long addend { 0 };
};

// One .data machine word. Unsigned so 80-bit x87 lanes keep their high bits.
struct StaticWord {
    unsigned long long bits { 0 };
};

using StaticInitValue = std::variant<StaticInteger, StaticFloat, StaticAddress, StaticWord>;

// Fold value to .data words. Floats and integers become StaticWord; addresses stay.
inline std::vector<StaticInitValue> asDataWords(const StaticInitValue& value) {
    if (const auto* fp = std::get_if<StaticFloat>(&value)) {
        if (fp->sizeBytes > 8) {
            return { StaticWord { fp->bits }, StaticWord { fp->bitsHi } };
        }
        return { StaticWord { fp->bits } };
    }
    if (const auto* integer = std::get_if<StaticInteger>(&value)) {
        const int words = type::object_abi::valueWords(integer->value.type.getSize());
        std::vector<StaticInitValue> out;
        out.reserve(static_cast<std::size_t>(words));
        for (int i = 0; i < words; ++i) {
            out.push_back(StaticWord { type::bitsWord(integer->value, i) });
        }
        return out;
    }
    return { value };
}

} // namespace symbols

#endif
