#include "SysVAbiInteropHarness.h"

// Variadic after register exhaustion: fixed args fill GP and/or SSE pools, then
// va_arg pulls from the reg-save / overflow area. Trans uses __builtin_va_*;
// gcc uses <stdarg.h>. Checksum returns; both call directions.

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
#define C_BIG R"prg(
        struct Big {
            long a;
            long b;
            long c;
        };
    )prg"

// --- B1: 6 fixed longs fill GPRs; two longs only in ... -------------------

constexpr const char* kVaAfterGpLibTrans = R"prg(
        long va_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            __builtin_va_list ap;
            long x, y;
            __builtin_va_start(ap, f);
            x = __builtin_va_arg(ap, long);
            y = __builtin_va_arg(ap, long);
            __builtin_va_end(ap);
            return a + b + c + d + e + f + x + y;
        }
    )prg";
constexpr const char* kVaAfterGpLibGcc = R"prg(
        #include <stdarg.h>
        long va_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            va_list ap;
            long x, y;
            va_start(ap, f);
            x = va_arg(ap, long);
            y = va_arg(ap, long);
            va_end(ap);
            return a + b + c + d + e + f + x + y;
        }
    )prg";
constexpr const char* kVaAfterGpMain = C_PRINTF R"prg(
        long va_after_gp(long, long, long, long, long, long, ...);
        int main(void) {
            printf("%d", (int)va_after_gp(1, 2, 3, 4, 5, 6, 100, 200));
            return 0;
        }
    )prg";

// --- B2: 8 fixed doubles fill XMM; two doubles only in ... ---------------

constexpr const char* kVaAfterSseLibTrans = R"prg(
        long va_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            __builtin_va_list ap;
            double x, y;
            __builtin_va_start(ap, h);
            x = __builtin_va_arg(ap, double);
            y = __builtin_va_arg(ap, double);
            __builtin_va_end(ap);
            return (long)(a + b + c + d + e + f + g + h + x + y);
        }
    )prg";
constexpr const char* kVaAfterSseLibGcc = R"prg(
        #include <stdarg.h>
        long va_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            va_list ap;
            double x, y;
            va_start(ap, h);
            x = va_arg(ap, double);
            y = va_arg(ap, double);
            va_end(ap);
            return (long)(a + b + c + d + e + f + g + h + x + y);
        }
    )prg";
constexpr const char* kVaAfterSseMain = C_PRINTF R"prg(
        long va_after_sse(double, double, double, double, double, double, double, double, ...);
        int main(void) {
            printf("%d", (int)va_after_sse(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 10.0, 20.0));
            return 0;
        }
    )prg";

// --- B3: GP full fixed, MEMORY Big in ... --------------------------------

constexpr const char* kVaBigAfterGpLibTrans = C_BIG R"prg(
        long va_big_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            __builtin_va_list ap;
            struct Big s;
            __builtin_va_start(ap, f);
            s = __builtin_va_arg(ap, struct Big);
            __builtin_va_end(ap);
            return a + b + c + d + e + f + s.a + s.b + s.c;
        }
    )prg";
constexpr const char* kVaBigAfterGpLibGcc = R"prg(
        #include <stdarg.h>
    )prg" C_BIG R"prg(
        long va_big_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            va_list ap;
            struct Big s;
            va_start(ap, f);
            s = va_arg(ap, struct Big);
            va_end(ap);
            return a + b + c + d + e + f + s.a + s.b + s.c;
        }
    )prg";
constexpr const char* kVaBigAfterGpMain = C_PRINTF C_BIG R"prg(
        long va_big_after_gp(long, long, long, long, long, long, ...);
        struct Big arg;
        int main(void) {
            arg.a = 10;
            arg.b = 20;
            arg.c = 12;
            printf("%d", (int)va_big_after_gp(1, 2, 3, 4, 5, 6, arg));
            return 0;
        }
    )prg";

// --- B4: GP full fixed, two-eightbyte Pair in ... ------------------------

constexpr const char* kVaPairAfterGpLibTrans = C_PAIR R"prg(
        long va_pair_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            __builtin_va_list ap;
            struct Pair p;
            __builtin_va_start(ap, f);
            p = __builtin_va_arg(ap, struct Pair);
            __builtin_va_end(ap);
            return a + b + c + d + e + f + p.a + p.b;
        }
    )prg";
constexpr const char* kVaPairAfterGpLibGcc = R"prg(
        #include <stdarg.h>
    )prg" C_PAIR R"prg(
        long va_pair_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            va_list ap;
            struct Pair p;
            va_start(ap, f);
            p = va_arg(ap, struct Pair);
            va_end(ap);
            return a + b + c + d + e + f + p.a + p.b;
        }
    )prg";
constexpr const char* kVaPairAfterGpMain = C_PRINTF C_PAIR R"prg(
        long va_pair_after_gp(long, long, long, long, long, long, ...);
        struct Pair arg;
        int main(void) {
            arg.a = 11;
            arg.b = 22;
            printf("%d", (int)va_pair_after_gp(1, 2, 3, 4, 5, 6, arg));
            return 0;
        }
    )prg";

// --- B5: fixed long/double mix, then long + double in ... ----------------

constexpr const char* kVaMixedTailLibTrans = R"prg(
        long va_mixed_tail(long a, double x, long b, double y, long c, double z, ...) {
            __builtin_va_list ap;
            long t;
            double u;
            __builtin_va_start(ap, z);
            t = __builtin_va_arg(ap, long);
            u = __builtin_va_arg(ap, double);
            __builtin_va_end(ap);
            return a + b + c + t + (long)(x + y + z + u);
        }
    )prg";
constexpr const char* kVaMixedTailLibGcc = R"prg(
        #include <stdarg.h>
        long va_mixed_tail(long a, double x, long b, double y, long c, double z, ...) {
            va_list ap;
            long t;
            double u;
            va_start(ap, z);
            t = va_arg(ap, long);
            u = va_arg(ap, double);
            va_end(ap);
            return a + b + c + t + (long)(x + y + z + u);
        }
    )prg";
constexpr const char* kVaMixedTailMain = C_PRINTF R"prg(
        long va_mixed_tail(long, double, long, double, long, double, ...);
        int main(void) {
            printf("%d", (int)va_mixed_tail(1, 2.0, 3, 4.0, 5, 6.0, 100, 10.0));
            return 0;
        }
    )prg";

// --- B6: SSE full fixed, MEMORY Big in ... -------------------------------

constexpr const char* kVaBigAfterSseLibTrans = C_BIG R"prg(
        long va_big_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            __builtin_va_list ap;
            struct Big s;
            __builtin_va_start(ap, h);
            s = __builtin_va_arg(ap, struct Big);
            __builtin_va_end(ap);
            return (long)(a + b + c + d + e + f + g + h) + s.a + s.b + s.c;
        }
    )prg";
constexpr const char* kVaBigAfterSseLibGcc = R"prg(
        #include <stdarg.h>
    )prg" C_BIG R"prg(
        long va_big_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            va_list ap;
            struct Big s;
            va_start(ap, h);
            s = va_arg(ap, struct Big);
            va_end(ap);
            return (long)(a + b + c + d + e + f + g + h) + s.a + s.b + s.c;
        }
    )prg";
constexpr const char* kVaBigAfterSseMain = C_PRINTF C_BIG R"prg(
        long va_big_after_sse(double, double, double, double, double, double, double, double, ...);
        struct Big arg;
        int main(void) {
            arg.a = 10;
            arg.b = 20;
            arg.c = 12;
            printf("%d", (int)va_big_after_sse(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, arg));
            return 0;
        }
    )prg";

// --- B7: SSE full fixed, two-eightbyte Pair in ... -----------------------

constexpr const char* kVaPairAfterSseLibTrans = C_PAIR R"prg(
        long va_pair_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            __builtin_va_list ap;
            struct Pair p;
            __builtin_va_start(ap, h);
            p = __builtin_va_arg(ap, struct Pair);
            __builtin_va_end(ap);
            return (long)(a + b + c + d + e + f + g + h) + p.a + p.b;
        }
    )prg";
constexpr const char* kVaPairAfterSseLibGcc = R"prg(
        #include <stdarg.h>
    )prg" C_PAIR R"prg(
        long va_pair_after_sse(double a, double b, double c, double d,
                double e, double f, double g, double h, ...) {
            va_list ap;
            struct Pair p;
            va_start(ap, h);
            p = va_arg(ap, struct Pair);
            va_end(ap);
            return (long)(a + b + c + d + e + f + g + h) + p.a + p.b;
        }
    )prg";
constexpr const char* kVaPairAfterSseMain = C_PRINTF C_PAIR R"prg(
        long va_pair_after_sse(double, double, double, double, double, double, double, double, ...);
        struct Pair arg;
        int main(void) {
            arg.a = 11;
            arg.b = 22;
            printf("%d", (int)va_pair_after_sse(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, arg));
            return 0;
        }
    )prg";

// --- B8: GP full fixed; narrow types in ... (default-arg-promoted to int) -

constexpr const char* kVaNarrowAfterGpLibTrans = R"prg(
        long va_narrow_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            __builtin_va_list ap;
            int x, y, z;
            __builtin_va_start(ap, f);
            x = __builtin_va_arg(ap, int);
            y = __builtin_va_arg(ap, int);
            z = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return a + b + c + d + e + f + x + y + z;
        }
    )prg";
constexpr const char* kVaNarrowAfterGpLibGcc = R"prg(
        #include <stdarg.h>
        long va_narrow_after_gp(long a, long b, long c, long d, long e, long f, ...) {
            va_list ap;
            int x, y, z;
            va_start(ap, f);
            x = va_arg(ap, int);
            y = va_arg(ap, int);
            z = va_arg(ap, int);
            va_end(ap);
            return a + b + c + d + e + f + x + y + z;
        }
    )prg";
constexpr const char* kVaNarrowAfterGpMain = C_PRINTF R"prg(
        long va_narrow_after_gp(long, long, long, long, long, long, ...);
        int main(void) {
            char ch;
            short sh;
            _Bool bo;
            ch = -5;
            sh = -300;
            bo = 1;
            printf("%d", (int)va_narrow_after_gp(1, 2, 3, 4, 5, 6, ch, sh, bo));
            return 0;
        }
    )prg";

#undef C_PRINTF
#undef C_PAIR
#undef C_BIG

SYSV_BOTH_LIBS(vaPressureAfterGp, "sysv_va_gp", kVaAfterGpLibTrans, kVaAfterGpLibGcc, kVaAfterGpMain, "321")
SYSV_BOTH_LIBS(vaPressureAfterSse, "sysv_va_sse", kVaAfterSseLibTrans, kVaAfterSseLibGcc, kVaAfterSseMain, "66")
SYSV_BOTH_LIBS(vaPressureBigAfterGp, "sysv_va_big_gp", kVaBigAfterGpLibTrans, kVaBigAfterGpLibGcc, kVaBigAfterGpMain, "63")
SYSV_BOTH_LIBS(vaPressurePairAfterGp, "sysv_va_pair_gp", kVaPairAfterGpLibTrans, kVaPairAfterGpLibGcc, kVaPairAfterGpMain, "54")
SYSV_BOTH_LIBS(vaPressureMixedTail, "sysv_va_mix_tail", kVaMixedTailLibTrans, kVaMixedTailLibGcc, kVaMixedTailMain, "131")
SYSV_BOTH_LIBS(vaPressureBigAfterSse, "sysv_va_big_sse", kVaBigAfterSseLibTrans, kVaBigAfterSseLibGcc, kVaBigAfterSseMain, "78")
SYSV_BOTH_LIBS(vaPressurePairAfterSse, "sysv_va_pair_sse", kVaPairAfterSseLibTrans, kVaPairAfterSseLibGcc, kVaPairAfterSseMain, "69")
SYSV_BOTH_LIBS(vaPressureNarrowAfterGp, "sysv_va_narrow_gp", kVaNarrowAfterGpLibTrans, kVaNarrowAfterGpLibGcc,
        kVaNarrowAfterGpMain, "-283")

} // namespace
