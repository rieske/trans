#ifndef CODEGEN_SSE_H_
#define CODEGEN_SSE_H_

#include <stdexcept>

namespace codegen {

// Scalar SSE width: binary32 vs binary64. Memory and xmm moves use this, not a
// float-type boolean.
enum class SseWidth { F32, F64 };

enum class SseBin { Add, Sub, Mul, Div };

enum class SseGprXmmDir { GprToXmm, XmmToGpr };

// Shared *ss / *sd names. Dialects only format operands.
inline const char* sseBinMnemonic(SseBin op, SseWidth width) {
    static const char* sd[] = { "addsd", "subsd", "mulsd", "divsd" };
    static const char* ss[] = { "addss", "subss", "mulss", "divss" };
    const int i = static_cast<int>(op);
    if (i < 0 || i > 3) {
        throw std::runtime_error { "sseBinMnemonic: unknown SseBin" };
    }
    return (width == SseWidth::F32 ? ss : sd)[i];
}

// True when converting float32 -> float64. Identical widths are a caller bug.
inline bool sseCvtFloatWidens(SseWidth from, SseWidth to) {
    if (from == to) {
        throw std::runtime_error { "sseCvtFloat: identical widths" };
    }
    return from == SseWidth::F32 && to == SseWidth::F64;
}

} // namespace codegen

#endif // CODEGEN_SSE_H_
