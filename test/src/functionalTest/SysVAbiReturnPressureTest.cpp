#include "SysVAbiInteropHarness.h"

// Return-side pressure: busy arg lists while returning INTEGER/SSE two-eightbyte
// results or MEMORY via sret. Field checksums; both directions.
//
// C bodies use C++11 raw string literals R"prg(...)prg" (see also Aggregates/Int128).
// Shared headers are macros so adjacent-literal concatenation works at compile time
// (a const char* fragment cannot concatenate with a string literal).

namespace {

#define C_PRINTF R"prg(
        int printf(const char *, ...);
    )prg"
#define C_PAIR R"prg(
        struct Pair {
            long a;
            long b;
        };
    )prg"
#define C_FF R"prg(
        struct FF {
            float a;
            float b;
        };
    )prg"
#define C_BIG R"prg(
        struct Big {
            long a;
            long b;
            long c;
        };
    )prg"

// Six user GP args; return {long,long} in rax:rdx.
constexpr const char* kRetPairBusyLib = C_PAIR R"prg(
        struct Pair ret_pair_busy(long a, long b, long c, long d, long e, long f) {
            struct Pair p;
            p.a = a + e;
            p.b = b + f + c + d;
            return p;
        }
    )prg";
constexpr const char* kRetPairBusyMain = C_PRINTF C_PAIR R"prg(
        struct Pair ret_pair_busy(long, long, long, long, long, long);
        int main(void) {
            struct Pair p;
            p = ret_pair_busy(1, 2, 3, 4, 5, 6);
            printf("%d %d", (int)p.a, (int)p.b);
            return 0;
        }
    )prg";

// Eight doubles fill xmm0..7; return {float,float} in xmm0.
// Inputs are integer-valued so (float)/(int) casts stay bit-stable for the expect string.
constexpr const char* kRetFfBusyLib = C_FF R"prg(
        struct FF ret_ff_busy(double a, double b, double c, double d,
                double e, double f, double g, double h) {
            struct FF s;
            s.a = (float)(a + e);
            s.b = (float)(b + f + g + h + c + d);
            return s;
        }
    )prg";
constexpr const char* kRetFfBusyMain = C_PRINTF C_FF R"prg(
        struct FF ret_ff_busy(double, double, double, double, double, double, double, double);
        int main(void) {
            struct FF s;
            s = ret_ff_busy(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
            printf("%d %d", (int)s.a, (int)s.b);
            return 0;
        }
    )prg";

// MEMORY sret: hidden pointer uses one INTEGER slot (rdi on entry to callee).
// Five user longs keep the remaining GPRs busy around that sret setup.
constexpr const char* kRetBigBusyLib = C_BIG R"prg(
        struct Big ret_big_busy(long a, long b, long c, long d, long e) {
            struct Big s;
            s.a = a;
            s.b = b + c;
            s.c = d + e;
            return s;
        }
    )prg";
constexpr const char* kRetBigBusyMain = C_PRINTF C_BIG R"prg(
        struct Big ret_big_busy(long, long, long, long, long);
        int main(void) {
            struct Big s;
            s = ret_big_busy(1, 2, 3, 4, 5);
            printf("%d %d %d", (int)s.a, (int)s.b, (int)s.c);
            return 0;
        }
    )prg";

// Three {long,long} args fill six integer eightbytes; return another pair in rax:rdx.
constexpr const char* kRetPairAfterPairsLib = C_PAIR R"prg(
        struct Pair ret_pair_after_pairs(struct Pair p0, struct Pair p1, struct Pair p2) {
            struct Pair r;
            r.a = p0.a + p1.a + p2.a;
            r.b = p0.b + p1.b + p2.b;
            return r;
        }
    )prg";
constexpr const char* kRetPairAfterPairsMain = C_PRINTF C_PAIR R"prg(
        struct Pair ret_pair_after_pairs(struct Pair, struct Pair, struct Pair);
        struct Pair p0, p1, p2;
        int main(void) {
            struct Pair p;
            p0.a = 1;
            p0.b = 2;
            p1.a = 3;
            p1.b = 4;
            p2.a = 5;
            p2.b = 6;
            p = ret_pair_after_pairs(p0, p1, p2);
            printf("%d %d", (int)p.a, (int)p.b);
            return 0;
        }
    )prg";

// Like matrix sretAndMemoryArg (MEMORY in + MEMORY out), but also trails INTEGER
// regs so sret, by-ref MEMORY arg, and plain GPRs interact on one call.
constexpr const char* kRetBigFromBigRegsLib = C_BIG R"prg(
        struct Big ret_big_from_big_regs(struct Big x, long k, long m) {
            struct Big r;
            r.a = x.a + k;
            r.b = x.b + m;
            r.c = x.c + k + m;
            return r;
        }
    )prg";
constexpr const char* kRetBigFromBigRegsMain = C_PRINTF C_BIG R"prg(
        struct Big ret_big_from_big_regs(struct Big, long, long);
        struct Big x;
        int main(void) {
            struct Big s;
            x.a = 10;
            x.b = 20;
            x.c = 30;
            s = ret_big_from_big_regs(x, 1, 2);
            printf("%d %d %d", (int)s.a, (int)s.b, (int)s.c);
            return 0;
        }
    )prg";

#undef C_PRINTF
#undef C_PAIR
#undef C_FF
#undef C_BIG

SYSV_BOTH(retPressurePairAfterGpArgs, "sysv_ret_pair_gp", kRetPairBusyLib, kRetPairBusyMain, "6 15")
SYSV_BOTH(retPressureFfAfterSseArgs, "sysv_ret_ff_sse", kRetFfBusyLib, kRetFfBusyMain, "6 30")
SYSV_BOTH(retPressureBigSretBusyArgs, "sysv_ret_big_busy", kRetBigBusyLib, kRetBigBusyMain, "1 5 9")
SYSV_BOTH(retPressurePairAfterPairArgs, "sysv_ret_pair_pairs", kRetPairAfterPairsLib, kRetPairAfterPairsMain, "9 12")
SYSV_BOTH(retPressureBigSretMemAndRegs, "sysv_ret_big_mem", kRetBigFromBigRegsLib, kRetBigFromBigRegsMain, "11 22 33")

} // namespace
