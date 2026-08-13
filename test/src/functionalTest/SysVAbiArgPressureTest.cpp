#include "SysVAbiInteropHarness.h"

// Argument-slot pressure: mixed INTEGER/SSE pools, first spill, multi-eightbyte
// aggregates, and MEMORY mixed with registers. Checksum return; both directions.

namespace {

// 6 longs fill rdi..r9; doubles still use xmm0/xmm1.
constexpr const char* kGpFullSseFreeLib =
        "long mix_gp_full(long a, long b, long c, long d, long e, long f,\n"
        "        double x, double y) {\n"
        "  return a + b + c + d + e + f + (long)(x + y);\n"
        "}\n";
constexpr const char* kGpFullSseFreeMain =
        "int printf(const char *, ...);\n"
        "long mix_gp_full(long, long, long, long, long, long, double, double);\n"
        "int main(void) {\n"
        "  printf(\"%d\", (int)mix_gp_full(1, 2, 3, 4, 5, 6, 10.0, 20.0));\n"
        "  return 0;\n"
        "}\n";

// 8 doubles fill xmm0..7; longs still use GPRs.
constexpr const char* kSseFullGpFreeLib =
        "long mix_sse_full(double a, double b, double c, double d,\n"
        "        double e, double f, double g, double h, long i, long j) {\n"
        "  return (long)(a + b + c + d + e + f + g + h) + i + j;\n"
        "}\n";
constexpr const char* kSseFullGpFreeMain =
        "int printf(const char *, ...);\n"
        "long mix_sse_full(double, double, double, double, double, double, double, double,\n"
        "        long, long);\n"
        "int main(void) {\n"
        "  printf(\"%d\", (int)mix_sse_full(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 100, 200));\n"
        "  return 0;\n"
        "}\n";

// 6 longs in regs; 7th integer is first stack spill.
constexpr const char* kFirstGprSpillLib =
        "long first_gpr_spill(long a, long b, long c, long d, long e, long f,\n"
        "        long stack_only) {\n"
        "  return a + b + c + d + e + f + stack_only;\n"
        "}\n";
constexpr const char* kFirstGprSpillMain =
        "int printf(const char *, ...);\n"
        "long first_gpr_spill(long, long, long, long, long, long, long);\n"
        "int main(void) {\n"
        "  printf(\"%d\", (int)first_gpr_spill(1, 2, 3, 4, 5, 6, 100));\n"
        "  return 0;\n"
        "}\n";

// 3x {long,long} uses 6 integer eightbytes; trailing long is on stack.
constexpr const char* kPairsThenStackLib =
        "struct Pair { long a; long b; };\n"
        "long pairs_then_stack(struct Pair p0, struct Pair p1, struct Pair p2,\n"
        "        long stack_long) {\n"
        "  return p0.a + p0.b + p1.a + p1.b + p2.a + p2.b + stack_long;\n"
        "}\n";
constexpr const char* kPairsThenStackMain =
        "int printf(const char *, ...);\n"
        "struct Pair { long a; long b; };\n"
        "long pairs_then_stack(struct Pair, struct Pair, struct Pair, long);\n"
        "struct Pair p0, p1, p2;\n"
        "int main(void) {\n"
        "  p0.a = 1; p0.b = 2; p1.a = 3; p1.b = 4; p2.a = 5; p2.b = 6;\n"
        "  printf(\"%d\", (int)pairs_then_stack(p0, p1, p2, 100));\n"
        "  return 0;\n"
        "}\n";

// MEMORY Big among INTEGER args (regs still free around the by-ref arg).
constexpr const char* kMemAmongRegsLib =
        "struct Big { long a; long b; long c; };\n"
        "long mem_among_regs(long r0, long r1, struct Big big, long r2) {\n"
        "  return r0 + r1 + big.a + big.b + big.c + r2;\n"
        "}\n";
constexpr const char* kMemAmongRegsMain =
        "int printf(const char *, ...);\n"
        "struct Big { long a; long b; long c; };\n"
        "long mem_among_regs(long, long, struct Big, long);\n"
        "struct Big big;\n"
        "int main(void) {\n"
        "  big.a = 10; big.b = 20; big.c = 12;\n"
        "  printf(\"%d\", (int)mem_among_regs(1, 2, big, 100));\n"
        "  return 0;\n"
        "}\n";

// Interleaved long/double past both register pools (GP and SSE spill).
constexpr const char* kInterleavedSpillLib =
        "long interleaved_spill(\n"
        "        long a0, double d0, long a1, double d1, long a2, double d2,\n"
        "        long a3, double d3, long a4, double d4, long a5, double d5,\n"
        "        double d6, double d7, double d8, long a6) {\n"
        "  return a0 + a1 + a2 + a3 + a4 + a5 + a6\n"
        "          + (long)(d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);\n"
        "}\n";
constexpr const char* kInterleavedSpillMain =
        "int printf(const char *, ...);\n"
        "long interleaved_spill(\n"
        "        long, double, long, double, long, double,\n"
        "        long, double, long, double, long, double,\n"
        "        double, double, double, long);\n"
        "int main(void) {\n"
        "  printf(\"%d\", (int)interleaved_spill(\n"
        "          1, 1.0, 2, 2.0, 3, 3.0, 4, 4.0, 5, 5.0, 6, 6.0,\n"
        "          7.0, 8.0, 9.0, 100));\n"
        "  return 0;\n"
        "}\n";

SYSV_BOTH(argPressureGpFullSseFree, "sysv_arg_gp_sse", kGpFullSseFreeLib, kGpFullSseFreeMain, "51")
SYSV_BOTH(argPressureSseFullGpFree, "sysv_arg_sse_gp", kSseFullGpFreeLib, kSseFullGpFreeMain, "336")
SYSV_BOTH(argPressureFirstGprSpill, "sysv_arg_gpr_spill", kFirstGprSpillLib, kFirstGprSpillMain, "121")
SYSV_BOTH(argPressurePairsThenStack, "sysv_arg_pairs_stk", kPairsThenStackLib, kPairsThenStackMain, "121")
SYSV_BOTH(argPressureMemAmongRegs, "sysv_arg_mem_regs", kMemAmongRegsLib, kMemAmongRegsMain, "145")
SYSV_BOTH(argPressureInterleavedSpill, "sysv_arg_interleave", kInterleavedSpillLib, kInterleavedSpillMain, "166")

} // namespace
