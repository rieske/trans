#ifndef TYPE_ICE_H_
#define TYPE_ICE_H_

#include <stdexcept>
#include <string>

namespace type {

[[noreturn]] inline void ice(const char* what) {
    throw std::logic_error { std::string { "internal compiler error: " } + what };
}

} // namespace type

#endif // TYPE_ICE_H_
