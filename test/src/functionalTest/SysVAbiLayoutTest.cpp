#include "SysVAbiInteropHarness.h"

// Eightbyte layout aggregates: INTEGER merge (int+float), nested records that
// stay in registers vs fall to MEMORY. Pass and return both directions.
// (two-float SSE and int+double INTEGER|SSE already locked in Aggregates/Matrix.)

namespace {

#define C_PRINTF R"prg(
        int printf(const char *, ...);
    )prg"
#define C_IF R"prg(
        struct IF {
            int i;
            float f;
        };
    )prg"
#define C_FI R"prg(
        struct FI {
            float f;
            int i;
        };
    )prg"
#define C_NESTED_IF R"prg(
        struct NestedIF {
            struct {
                int i;
                float f;
            } inner;
            int tag;
        };
    )prg"
#define C_NESTED_BIG R"prg(
        struct NestedBig {
            struct {
                long a;
                long b;
            } in;
            long c;
        };
    )prg"

// --- C1: int then float share one INTEGER eightbyte (pass) ---------------

constexpr const char* kIfPassLib = C_IF R"prg(
        int sum_if(struct IF s) {
            return s.i + (int)s.f;
        }
    )prg";
constexpr const char* kIfPassMain = C_PRINTF C_IF R"prg(
        int sum_if(struct IF);
        struct IF arg;
        int main(void) {
            arg.i = 3;
            arg.f = 4.0f;
            printf("%d", sum_if(arg));
            return 0;
        }
    )prg";

// --- C1r: same layout returned in registers ------------------------------

constexpr const char* kIfRetLib = C_IF R"prg(
        struct IF make_if(int i, float f) {
            struct IF s;
            s.i = i;
            s.f = f;
            return s;
        }
    )prg";
constexpr const char* kIfRetMain = C_PRINTF C_IF R"prg(
        struct IF make_if(int, float);
        int main(void) {
            struct IF s;
            s = make_if(10, 20.0f);
            printf("%d %d", s.i, (int)s.f);
            return 0;
        }
    )prg";

// --- C2: float then int (same size, field order) pass + return -----------

constexpr const char* kFiPassLib = C_FI R"prg(
        int sum_fi(struct FI s) {
            return (int)s.f + s.i;
        }
    )prg";
constexpr const char* kFiPassMain = C_PRINTF C_FI R"prg(
        int sum_fi(struct FI);
        struct FI arg;
        int main(void) {
            arg.f = 3.0f;
            arg.i = 4;
            printf("%d", sum_fi(arg));
            return 0;
        }
    )prg";

constexpr const char* kFiRetLib = C_FI R"prg(
        struct FI make_fi(float f, int i) {
            struct FI s;
            s.f = f;
            s.i = i;
            return s;
        }
    )prg";
constexpr const char* kFiRetMain = C_PRINTF C_FI R"prg(
        struct FI make_fi(float, int);
        int main(void) {
            struct FI s;
            s = make_fi(10.0f, 20);
            printf("%d %d", (int)s.f, s.i);
            return 0;
        }
    )prg";

// --- C3: nested IF + tag stays <=16B (register class) pass + return ------

constexpr const char* kNestedIfPassLib = C_NESTED_IF R"prg(
        int sum_nested_if(struct NestedIF n) {
            return n.inner.i + (int)n.inner.f + n.tag;
        }
    )prg";
constexpr const char* kNestedIfPassMain = C_PRINTF C_NESTED_IF R"prg(
        int sum_nested_if(struct NestedIF);
        struct NestedIF arg;
        int main(void) {
            arg.inner.i = 1;
            arg.inner.f = 2.0f;
            arg.tag = 3;
            printf("%d", sum_nested_if(arg));
            return 0;
        }
    )prg";

constexpr const char* kNestedIfRetLib = C_NESTED_IF R"prg(
        struct NestedIF make_nested_if(int i, float f, int tag) {
            struct NestedIF n;
            n.inner.i = i;
            n.inner.f = f;
            n.tag = tag;
            return n;
        }
    )prg";
constexpr const char* kNestedIfRetMain = C_PRINTF C_NESTED_IF R"prg(
        struct NestedIF make_nested_if(int, float, int);
        int main(void) {
            struct NestedIF n;
            n = make_nested_if(4, 5.0f, 6);
            printf("%d %d %d", n.inner.i, (int)n.inner.f, n.tag);
            return 0;
        }
    )prg";

// --- C4: nested three-long shape is MEMORY (pass + sret return) ----------

constexpr const char* kNestedBigPassLib = C_NESTED_BIG R"prg(
        long sum_nested_big(struct NestedBig n) {
            return n.in.a + n.in.b + n.c;
        }
    )prg";
constexpr const char* kNestedBigPassMain = C_PRINTF C_NESTED_BIG R"prg(
        long sum_nested_big(struct NestedBig);
        struct NestedBig arg;
        int main(void) {
            arg.in.a = 1;
            arg.in.b = 2;
            arg.c = 3;
            printf("%d", (int)sum_nested_big(arg));
            return 0;
        }
    )prg";

constexpr const char* kNestedBigRetLib = C_NESTED_BIG R"prg(
        struct NestedBig make_nested_big(long a, long b, long c) {
            struct NestedBig n;
            n.in.a = a;
            n.in.b = b;
            n.c = c;
            return n;
        }
    )prg";
constexpr const char* kNestedBigRetMain = C_PRINTF C_NESTED_BIG R"prg(
        struct NestedBig make_nested_big(long, long, long);
        int main(void) {
            struct NestedBig n;
            n = make_nested_big(10, 20, 12);
            printf("%d %d %d", (int)n.in.a, (int)n.in.b, (int)n.c);
            return 0;
        }
    )prg";

#undef C_PRINTF
#undef C_IF
#undef C_FI
#undef C_NESTED_IF
#undef C_NESTED_BIG

SYSV_BOTH(layoutIfPass, "sysv_layout_if_pass", kIfPassLib, kIfPassMain, "7")
SYSV_BOTH(layoutIfReturn, "sysv_layout_if_ret", kIfRetLib, kIfRetMain, "10 20")
SYSV_BOTH(layoutFiPass, "sysv_layout_fi_pass", kFiPassLib, kFiPassMain, "7")
SYSV_BOTH(layoutFiReturn, "sysv_layout_fi_ret", kFiRetLib, kFiRetMain, "10 20")
SYSV_BOTH(layoutNestedIfPass, "sysv_layout_nif_pass", kNestedIfPassLib, kNestedIfPassMain, "6")
SYSV_BOTH(layoutNestedIfReturn, "sysv_layout_nif_ret", kNestedIfRetLib, kNestedIfRetMain, "4 5 6")
SYSV_BOTH(layoutNestedBigPass, "sysv_layout_nbig_pass", kNestedBigPassLib, kNestedBigPassMain, "6")
SYSV_BOTH(layoutNestedBigReturn, "sysv_layout_nbig_ret", kNestedBigRetLib, kNestedBigRetMain, "10 20 12")

} // namespace
