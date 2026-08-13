#include "SysVAbiInteropHarness.h"

// Return-side pressure: busy arg lists while returning INTEGER/SSE two-eightbyte
// results or MEMORY via sret. Field/printf checksums; both directions.

namespace {

// Six GP args fill rdi..r9; return {long,long} in rax:rdx.
constexpr const char* kRetPairBusyLib =
        "struct Pair { long a; long b; };\n"
        "struct Pair ret_pair_busy(long a, long b, long c, long d, long e, long f) {\n"
        "  struct Pair p; p.a = a + e; p.b = b + f + c + d; return p;\n"
        "}\n";
constexpr const char* kRetPairBusyMain =
        "int printf(const char *, ...);\n"
        "struct Pair { long a; long b; };\n"
        "struct Pair ret_pair_busy(long, long, long, long, long, long);\n"
        "int main(void) {\n"
        "  struct Pair p; p = ret_pair_busy(1, 2, 3, 4, 5, 6);\n"
        "  printf(\"%d %d\", (int)p.a, (int)p.b);\n"
        "  return 0;\n"
        "}\n";

// Eight SSE args fill xmm0..7; return {float,float} in xmm0.
constexpr const char* kRetFfBusyLib =
        "struct FF { float a; float b; };\n"
        "struct FF ret_ff_busy(double a, double b, double c, double d,\n"
        "        double e, double f, double g, double h) {\n"
        "  struct FF s; s.a = (float)(a + e); s.b = (float)(b + f + g + h + c + d);\n"
        "  return s;\n"
        "}\n";
constexpr const char* kRetFfBusyMain =
        "int printf(const char *, ...);\n"
        "struct FF { float a; float b; };\n"
        "struct FF ret_ff_busy(double, double, double, double, double, double, double, double);\n"
        "int main(void) {\n"
        "  struct FF s; s = ret_ff_busy(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);\n"
        "  printf(\"%d %d\", (int)s.a, (int)s.b);\n"
        "  return 0;\n"
        "}\n";

// MEMORY sret while several INTEGER args occupy GPRs (hidden sret pointer + args).
constexpr const char* kRetBigBusyLib =
        "struct Big { long a; long b; long c; };\n"
        "struct Big ret_big_busy(long a, long b, long c, long d, long e) {\n"
        "  struct Big s; s.a = a; s.b = b + c; s.c = d + e; return s;\n"
        "}\n";
constexpr const char* kRetBigBusyMain =
        "int printf(const char *, ...);\n"
        "struct Big { long a; long b; long c; };\n"
        "struct Big ret_big_busy(long, long, long, long, long);\n"
        "int main(void) {\n"
        "  struct Big s; s = ret_big_busy(1, 2, 3, 4, 5);\n"
        "  printf(\"%d %d %d\", (int)s.a, (int)s.b, (int)s.c);\n"
        "  return 0;\n"
        "}\n";

// Three {long,long} args fill integer slots; return another pair in rax:rdx.
constexpr const char* kRetPairAfterPairsLib =
        "struct Pair { long a; long b; };\n"
        "struct Pair ret_pair_after_pairs(struct Pair p0, struct Pair p1, struct Pair p2) {\n"
        "  struct Pair r; r.a = p0.a + p1.a + p2.a; r.b = p0.b + p1.b + p2.b; return r;\n"
        "}\n";
constexpr const char* kRetPairAfterPairsMain =
        "int printf(const char *, ...);\n"
        "struct Pair { long a; long b; };\n"
        "struct Pair ret_pair_after_pairs(struct Pair, struct Pair, struct Pair);\n"
        "struct Pair p0, p1, p2;\n"
        "int main(void) {\n"
        "  struct Pair p;\n"
        "  p0.a = 1; p0.b = 2; p1.a = 3; p1.b = 4; p2.a = 5; p2.b = 6;\n"
        "  p = ret_pair_after_pairs(p0, p1, p2);\n"
        "  printf(\"%d %d\", (int)p.a, (int)p.b);\n"
        "  return 0;\n"
        "}\n";

// sret MEMORY return from MEMORY arg plus trailing INTEGER regs.
constexpr const char* kRetBigFromBigRegsLib =
        "struct Big { long a; long b; long c; };\n"
        "struct Big ret_big_from_big_regs(struct Big x, long k, long m) {\n"
        "  struct Big r; r.a = x.a + k; r.b = x.b + m; r.c = x.c + k + m; return r;\n"
        "}\n";
constexpr const char* kRetBigFromBigRegsMain =
        "int printf(const char *, ...);\n"
        "struct Big { long a; long b; long c; };\n"
        "struct Big ret_big_from_big_regs(struct Big, long, long);\n"
        "struct Big x;\n"
        "int main(void) {\n"
        "  struct Big s;\n"
        "  x.a = 10; x.b = 20; x.c = 30;\n"
        "  s = ret_big_from_big_regs(x, 1, 2);\n"
        "  printf(\"%d %d %d\", (int)s.a, (int)s.b, (int)s.c);\n"
        "  return 0;\n"
        "}\n";

SYSV_BOTH(retPressurePairAfterGpArgs, "sysv_ret_pair_gp", kRetPairBusyLib, kRetPairBusyMain, "6 15")
SYSV_BOTH(retPressureFfAfterSseArgs, "sysv_ret_ff_sse", kRetFfBusyLib, kRetFfBusyMain, "6 30")
SYSV_BOTH(retPressureBigSretBusyArgs, "sysv_ret_big_busy", kRetBigBusyLib, kRetBigBusyMain, "1 5 9")
SYSV_BOTH(retPressurePairAfterPairArgs, "sysv_ret_pair_pairs", kRetPairAfterPairsLib, kRetPairAfterPairsMain, "9 12")
SYSV_BOTH(retPressureBigSretMemAndRegs, "sysv_ret_big_mem", kRetBigFromBigRegsLib, kRetBigFromBigRegsMain, "11 22 33")

} // namespace
