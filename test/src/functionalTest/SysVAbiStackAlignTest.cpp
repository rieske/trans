#include "SysVAbiInteropHarness.h"

// Stack alignment after spill: odd/even stack eightbytes before 16-byte-aligned
// long double, MEMORY among/after GP fill, SSE full then stack double + ld.
// Integer-valued long doubles so (long) casts stay stable. Both directions.

namespace {

#define C_PRINTF R"prg(
        int printf(const char *, ...);
    )prg"
#define C_BIG R"prg(
        struct Big {
            long a;
            long b;
            long c;
        };
    )prg"
#define C_PAIR R"prg(
        struct Pair {
            long a;
            long b;
        };
    )prg"

// --- D1: GP full, one stack long (odd 8B), then 16B-aligned long double ----

constexpr const char* kOddStackThenLdLib = R"prg(
        long odd_stack_then_ld(long a, long b, long c, long d, long e, long f,
                long g, long double ld) {
            return a + b + c + d + e + f + g + (long)ld;
        }
    )prg";
constexpr const char* kOddStackThenLdMain = C_PRINTF R"prg(
        long odd_stack_then_ld(long, long, long, long, long, long, long, long double);
        int main(void) {
            printf("%d", (int)odd_stack_then_ld(1, 2, 3, 4, 5, 6, 7, 100.0L));
            return 0;
        }
    )prg";

// --- D2: GP full, two stack longs (even 16B), then long double -------------

constexpr const char* kEvenStackThenLdLib = R"prg(
        long even_stack_then_ld(long a, long b, long c, long d, long e, long f,
                long g, long h, long double ld) {
            return a + b + c + d + e + f + g + h + (long)ld;
        }
    )prg";
constexpr const char* kEvenStackThenLdMain = C_PRINTF R"prg(
        long even_stack_then_ld(long, long, long, long, long, long, long, long, long double);
        int main(void) {
            printf("%d", (int)even_stack_then_ld(1, 2, 3, 4, 5, 6, 7, 8, 100.0L));
            return 0;
        }
    )prg";

// --- D3: MEMORY Big mid-list (by-ref in INTEGER reg) + trailing stack long -

constexpr const char* kMemMidThenStackLib = C_BIG R"prg(
        long mem_mid_then_stack(long a, long b, long c, long d, long e,
                struct Big big, long stack) {
            return a + b + c + d + e + big.a + big.b + big.c + stack;
        }
    )prg";
constexpr const char* kMemMidThenStackMain = C_PRINTF C_BIG R"prg(
        long mem_mid_then_stack(long, long, long, long, long, struct Big, long);
        struct Big big;
        int main(void) {
            big.a = 10;
            big.b = 20;
            big.c = 12;
            printf("%d", (int)mem_mid_then_stack(1, 2, 3, 4, 5, big, 100));
            return 0;
        }
    )prg";

// --- D4: GP full, MEMORY Big on stack, trailing long ---------------------

constexpr const char* kMemAfterGpSpillLib = C_BIG R"prg(
        long mem_after_gp_spill(long a, long b, long c, long d, long e, long f,
                struct Big big, long stack) {
            return a + b + c + d + e + f + big.a + big.b + big.c + stack;
        }
    )prg";
constexpr const char* kMemAfterGpSpillMain = C_PRINTF C_BIG R"prg(
        long mem_after_gp_spill(long, long, long, long, long, long, struct Big, long);
        struct Big big;
        int main(void) {
            big.a = 10;
            big.b = 20;
            big.c = 12;
            printf("%d", (int)mem_after_gp_spill(1, 2, 3, 4, 5, 6, big, 100));
            return 0;
        }
    )prg";

// --- D5: SSE full, stack double, then long double ------------------------

constexpr const char* kSseSpillThenLdLib = R"prg(
        long sse_spill_then_ld(double a, double b, double c, double d,
                double e, double f, double g, double h, double i, long double ld) {
            return (long)(a + b + c + d + e + f + g + h + i + ld);
        }
    )prg";
constexpr const char* kSseSpillThenLdMain = C_PRINTF R"prg(
        long sse_spill_then_ld(double, double, double, double, double, double, double, double,
                double, long double);
        int main(void) {
            printf("%d", (int)sse_spill_then_ld(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 100.0L));
            return 0;
        }
    )prg";

// --- D6: three pairs fill INTEGER slots; long double on stack ------------

constexpr const char* kPairsThenLdLib = C_PAIR R"prg(
        long pairs_then_ld(struct Pair p0, struct Pair p1, struct Pair p2, long double ld) {
            return p0.a + p0.b + p1.a + p1.b + p2.a + p2.b + (long)ld;
        }
    )prg";
constexpr const char* kPairsThenLdMain = C_PRINTF C_PAIR R"prg(
        long pairs_then_ld(struct Pair, struct Pair, struct Pair, long double);
        struct Pair p0, p1, p2;
        int main(void) {
            p0.a = 1;
            p0.b = 2;
            p1.a = 3;
            p1.b = 4;
            p2.a = 5;
            p2.b = 6;
            printf("%d", (int)pairs_then_ld(p0, p1, p2, 100.0L));
            return 0;
        }
    )prg";

#undef C_PRINTF
#undef C_BIG
#undef C_PAIR

SYSV_BOTH(stackAlignOddThenLd, "sysv_stk_odd_ld", kOddStackThenLdLib, kOddStackThenLdMain, "128")
SYSV_BOTH(stackAlignEvenThenLd, "sysv_stk_even_ld", kEvenStackThenLdLib, kEvenStackThenLdMain, "136")
SYSV_BOTH(stackAlignMemMidThenStack, "sysv_stk_mem_mid", kMemMidThenStackLib, kMemMidThenStackMain, "157")
SYSV_BOTH(stackAlignMemAfterGpSpill, "sysv_stk_mem_spill", kMemAfterGpSpillLib, kMemAfterGpSpillMain, "163")
SYSV_BOTH(stackAlignSseSpillThenLd, "sysv_stk_sse_ld", kSseSpillThenLdLib, kSseSpillThenLdMain, "145")
SYSV_BOTH(stackAlignPairsThenLd, "sysv_stk_pairs_ld", kPairsThenLdLib, kPairsThenLdMain, "121")

} // namespace
