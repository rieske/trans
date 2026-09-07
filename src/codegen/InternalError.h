#ifndef INTERNALERROR_H_
#define INTERNALERROR_H_

#include <stdexcept>
#include <string>

namespace codegen {

// Semantic analysis publishes what code generation reads. A missing one is a compiler
// bug rather than a user diagnostic, so it must fail in release builds too.
[[noreturn]] inline void internalError(const std::string& what) {
    throw std::logic_error { "internal compiler error: " + what };
}

inline void require(const void* value, const char* what) {
    if (!value) {
        internalError(std::string { what } + " is missing");
    }
}

} // namespace codegen

#endif // INTERNALERROR_H_
