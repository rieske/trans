#ifndef SYMBOLS_GLOBALINITIALIZER_H_
#define SYMBOLS_GLOBALINITIALIZER_H_

#include <string>
#include <variant>
#include <vector>

namespace symbols {

struct ConstantInit {
    long value { 0 };
};
struct AddressInit {
    std::string symbol;
    long offset { 0 };
};
using DataWord = std::variant<ConstantInit, AddressInit>;
struct MultiWordInit {
    std::vector<DataWord> words;
};

using GlobalInitializer = std::variant<ConstantInit, AddressInit, MultiWordInit>;

} // namespace symbols

#endif
