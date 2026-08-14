#include "SysVAbiInteropHarness.h"

// Packed aggregates: unaligned int after char is MEMORY (size 5) on SysV.

namespace {

#define C_PRINTF R"prg(
        int printf(const char *, ...);
    )prg"
#define C_PACKED_CI R"prg(
        struct __attribute__((packed)) S {
            char c;
            int i;
        };
    )prg"
#define C_PACKED_SI R"prg(
        struct __attribute__((packed)) S {
            short s;
            int i;
        };
    )prg"

constexpr const char* kPackedCiPassLib = C_PACKED_CI R"prg(
        int take_s(struct S s) {
            return s.c + s.i;
        }
    )prg";
constexpr const char* kPackedCiPassMain = C_PRINTF C_PACKED_CI R"prg(
        int take_s(struct S);
        int main(void) {
            struct S s;
            s.c = 1;
            s.i = 40;
            printf("%d", take_s(s));
            return 0;
        }
    )prg";

constexpr const char* kPackedCiRetLib = C_PACKED_CI R"prg(
        struct S make_s(void) {
            struct S s;
            s.c = 1;
            s.i = 40;
            return s;
        }
    )prg";
constexpr const char* kPackedCiRetMain = C_PRINTF C_PACKED_CI R"prg(
        struct S make_s(void);
        int main(void) {
            struct S s;
            s = make_s();
            printf("%d %d", s.c, s.i);
            return 0;
        }
    )prg";

constexpr const char* kPackedSiPassLib = C_PACKED_SI R"prg(
        int take_s(struct S s) {
            return s.s + s.i;
        }
    )prg";
constexpr const char* kPackedSiPassMain = C_PRINTF C_PACKED_SI R"prg(
        int take_s(struct S);
        int main(void) {
            struct S s;
            s.s = 2;
            s.i = 40;
            printf("%d", take_s(s));
            return 0;
        }
    )prg";

#undef C_PRINTF
#undef C_PACKED_CI
#undef C_PACKED_SI

SYSV_BOTH(packedCharIntPass, "sysv_pack_ci_pass", kPackedCiPassLib, kPackedCiPassMain, "41")
SYSV_BOTH(packedCharIntReturn, "sysv_pack_ci_ret", kPackedCiRetLib, kPackedCiRetMain, "1 40")
SYSV_BOTH(packedShortIntPass, "sysv_pack_si_pass", kPackedSiPassLib, kPackedSiPassMain, "42")

} // namespace
