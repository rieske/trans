#include "SysVAbiInteropHarness.h"

// Compact MEMORY/sret/SSE/stack/va/aggregate locks. Shared C fragments; both directions default.

namespace {

// Adjacent string literals concatenate; keep types/mains DRY.
#define C_PRINTF "int printf(const char *, ...);\n"
#define C_BIG "struct Big { long a; long b; long c; };\n"
#define C_PAIR "struct Pair { long a; long b; };\n"
#define C_FF "struct FF { float a; float b; };\n"
#define C_F4 "struct F4 { float a; float b; float c; float d; };\n"
#define C_DI "struct DI { double d; int i; };\n"
#define C_T3 "struct T { char a; char b; char c; };\n"
#define C_INNER "struct Inner { long x; long y; };\n"
#define C_OUTER "struct Outer { struct Inner in; int z; };\n"
#define C_UNION "union U { long l; double d; };\n"
#define C_FPTR_S "struct S { int (*fp)(int); int x; };\n"

// --- P0 ------------------------------------------------------------------

constexpr const char* kMemPassLib = C_BIG
        "long sum_big(struct Big s) { return s.a + s.b + s.c; }\n";
constexpr const char* kMemPassMain = C_PRINTF C_BIG
        "long sum_big(struct Big);\n"
        "struct Big arg;\n"
        "int main(void) {\n"
        "  arg.a = 1; arg.b = 2; arg.c = 3;\n"
        "  printf(\"%d\", (int)sum_big(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kMemRetLib = C_BIG
        "struct Big make_big(void) {\n"
        "  struct Big s; s.a = 1; s.b = 2; s.c = 3; return s;\n"
        "}\n";
constexpr const char* kMemRetMain = C_PRINTF C_BIG
        "struct Big make_big(void);\n"
        "int main(void) {\n"
        "  struct Big s; s = make_big();\n"
        "  printf(\"%d %d %d\", (int)s.a, (int)s.b, (int)s.c);\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kSretMemLib = C_BIG
        "struct Big bump_big(struct Big x) {\n"
        "  struct Big r; r.a = x.a + 1; r.b = x.b + 1; r.c = x.c + 1; return r;\n"
        "}\n";
constexpr const char* kSretMemMain = C_PRINTF C_BIG
        "struct Big bump_big(struct Big);\n"
        "struct Big arg;\n"
        "int main(void) {\n"
        "  struct Big r; arg.a = 1; arg.b = 2; arg.c = 3; r = bump_big(arg);\n"
        "  printf(\"%d %d %d\", (int)r.a, (int)r.b, (int)r.c);\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kGpExhLib = C_PAIR
        "long sum4(struct Pair w, struct Pair x, struct Pair y, struct Pair z) {\n"
        "  return w.a + w.b + x.a + x.b + y.a + y.b + z.a + z.b;\n"
        "}\n";
constexpr const char* kGpExhMain = C_PRINTF C_PAIR
        "long sum4(struct Pair, struct Pair, struct Pair, struct Pair);\n"
        "struct Pair w, x, y, z;\n"
        "int main(void) {\n"
        "  w.a = 1; w.b = 2; x.a = 3; x.b = 4;\n"
        "  y.a = 5; y.b = 6; z.a = 7; z.b = 8;\n"
        "  printf(\"%d\", (int)sum4(w, x, y, z));\n"
        "  return 0;\n"
        "}\n";

// --- P1 ------------------------------------------------------------------

constexpr const char* kFfRetLib = C_FF
        "struct FF make_ff(void) { struct FF s; s.a = 20.0f; s.b = 22.0f; return s; }\n";
constexpr const char* kFfRetMain = C_PRINTF C_FF
        "struct FF make_ff(void);\n"
        "int main(void) {\n"
        "  struct FF s; s = make_ff();\n"
        "  printf(\"%d\", (int)(s.a + s.b));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kF4PassLib = C_F4
        "int sum_f4(struct F4 s) { return (int)(s.a + s.b + s.c + s.d); }\n";
constexpr const char* kF4PassMain = C_PRINTF C_F4
        "int sum_f4(struct F4);\n"
        "struct F4 arg;\n"
        "int main(void) {\n"
        "  arg.a = 1.0f; arg.b = 2.0f; arg.c = 3.0f; arg.d = 4.0f;\n"
        "  printf(\"%d\", sum_f4(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kF4RetLib = C_F4
        "struct F4 make_f4(void) {\n"
        "  struct F4 s; s.a = 1.0f; s.b = 2.0f; s.c = 3.0f; s.d = 4.0f; return s;\n"
        "}\n";
constexpr const char* kF4RetMain = C_PRINTF C_F4
        "struct F4 make_f4(void);\n"
        "int main(void) {\n"
        "  struct F4 s; s = make_f4();\n"
        "  printf(\"%d\", (int)(s.a + s.b + s.c + s.d));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kDiPassLib = C_DI
        "int sum_di(struct DI s) { return (int)s.d + s.i; }\n";
constexpr const char* kDiPassMain = C_PRINTF C_DI
        "int sum_di(struct DI);\n"
        "struct DI arg;\n"
        "int main(void) {\n"
        "  arg.d = 10.0; arg.i = 32;\n"
        "  printf(\"%d\", sum_di(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kVaMemLibTrans = C_BIG
        "long take_big(int n, ...) {\n"
        "  __builtin_va_list ap; struct Big s;\n"
        "  __builtin_va_start(ap, n);\n"
        "  s = __builtin_va_arg(ap, struct Big);\n"
        "  __builtin_va_end(ap);\n"
        "  return s.a + s.b + s.c;\n"
        "}\n";
constexpr const char* kVaMemLibGcc =
        "#include <stdarg.h>\n" C_BIG
        "long take_big(int n, ...) {\n"
        "  va_list ap; struct Big s;\n"
        "  va_start(ap, n);\n"
        "  s = va_arg(ap, struct Big);\n"
        "  va_end(ap);\n"
        "  return s.a + s.b + s.c;\n"
        "}\n";
constexpr const char* kVaMemMain = C_PRINTF C_BIG
        "long take_big(int n, ...);\n"
        "struct Big arg;\n"
        "int main(void) {\n"
        "  arg.a = 10; arg.b = 20; arg.c = 12;\n"
        "  printf(\"%d\", (int)take_big(0, arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kSse9Lib =
        "double sum9(double a, double b, double c, double d, double e, double f, double g, double h,\n"
        "        double i) {\n"
        "  return a + b + c + d + e + f + g + h + i;\n"
        "}\n";
constexpr const char* kSse9Main = C_PRINTF
        "double sum9(double, double, double, double, double, double, double, double, double);\n"
        "int main(void) {\n"
        "  printf(\"%d\", (int)sum9(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 6.0));\n"
        "  return 0;\n"
        "}\n";

// --- P2 ------------------------------------------------------------------

constexpr const char* kNestLib = C_INNER C_OUTER
        "long sum_outer(struct Outer o) { return o.in.x + o.in.y + o.z; }\n";
constexpr const char* kNestMain = C_PRINTF C_INNER C_OUTER
        "long sum_outer(struct Outer);\n"
        "struct Outer arg;\n"
        "int main(void) {\n"
        "  arg.in.x = 10; arg.in.y = 20; arg.z = 12;\n"
        "  printf(\"%d\", (int)sum_outer(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kUnionLib = C_UNION "long take_u(union U u) { return u.l; }\n";
constexpr const char* kUnionMain = C_PRINTF C_UNION
        "long take_u(union U);\n"
        "union U arg;\n"
        "int main(void) {\n"
        "  arg.l = 42;\n"
        "  printf(\"%d\", (int)take_u(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kTinyLib = C_T3 "int sum_t(struct T s) { return s.a + s.b + s.c; }\n";
constexpr const char* kTinyMain = C_PRINTF C_T3
        "int sum_t(struct T);\n"
        "struct T arg;\n"
        "int main(void) {\n"
        "  arg.a = 10; arg.b = 20; arg.c = 12;\n"
        "  printf(\"%d\", sum_t(arg));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kFptrArgLib =
        "int apply(int (*fp)(int, int), int a, int b) { return fp(a, b); }\n"
        "int add(int a, int b) { return a + b; }\n";
constexpr const char* kFptrArgMain = C_PRINTF
        "int apply(int (*)(int, int), int, int);\n"
        "int add(int, int);\n"
        "int main(void) {\n"
        "  printf(\"%d\", apply(add, 20, 22));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kFptrStructLib = C_FPTR_S
        "int apply_s(struct S s) { return s.fp(s.x); }\n"
        "int id(int x) { return x; }\n";
constexpr const char* kFptrStructMain = C_PRINTF C_FPTR_S
        "int apply_s(struct S);\n"
        "int id(int);\n"
        "int main(void) {\n"
        "  struct S s; s.fp = id; s.x = 42;\n"
        "  printf(\"%d\", apply_s(s));\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kVaPromLibTrans =
        "int take3(int n, ...) {\n"
        "  __builtin_va_list ap; int a, b, c;\n"
        "  __builtin_va_start(ap, n);\n"
        "  a = __builtin_va_arg(ap, int);\n"
        "  b = __builtin_va_arg(ap, int);\n"
        "  c = __builtin_va_arg(ap, int);\n"
        "  __builtin_va_end(ap);\n"
        "  return a + b + c;\n"
        "}\n";
constexpr const char* kVaPromLibGcc =
        "#include <stdarg.h>\n"
        "int take3(int n, ...) {\n"
        "  va_list ap; int a, b, c;\n"
        "  va_start(ap, n);\n"
        "  a = va_arg(ap, int);\n"
        "  b = va_arg(ap, int);\n"
        "  c = va_arg(ap, int);\n"
        "  va_end(ap);\n"
        "  return a + b + c;\n"
        "}\n";
constexpr const char* kVaPromMain = C_PRINTF
        "int take3(int n, ...);\n"
        "int main(void) {\n"
        "  char ch; short sh; _Bool b;\n"
        "  ch = -5; sh = -300; b = 1;\n"
        "  printf(\"%d\", take3(0, ch, sh, b));\n"
        "  return 0;\n"
        "}\n";

#undef C_PRINTF
#undef C_BIG
#undef C_PAIR
#undef C_FF
#undef C_F4
#undef C_DI
#undef C_T3
#undef C_INNER
#undef C_OUTER
#undef C_UNION
#undef C_FPTR_S

// P0
SYSV_BOTH(memoryThreeLongPass, "sysv_mem3_pass", kMemPassLib, kMemPassMain, "6")
SYSV_BOTH(memoryThreeLongReturn, "sysv_mem3_ret", kMemRetLib, kMemRetMain, "1 2 3")
SYSV_BOTH(sretAndMemoryArg, "sysv_sret_mem", kSretMemLib, kSretMemMain, "2 3 4")
SYSV_BOTH(gpExhaustFourPairs, "sysv_gp_exh", kGpExhLib, kGpExhMain, "36")

// P1
SYSV_BOTH(twoFloatStructReturn, "sysv_ff_ret", kFfRetLib, kFfRetMain, "42")
SYSV_BOTH(fourFloatStructPass, "sysv_f4_pass", kF4PassLib, kF4PassMain, "10")
SYSV_BOTH(fourFloatStructReturn, "sysv_f4_ret", kF4RetLib, kF4RetMain, "10")
SYSV_BOTH(doubleThenIntPass, "sysv_di_pass", kDiPassLib, kDiPassMain, "42")
SYSV_BOTH(sseExhaustNineDoubles, "sysv_sse9", kSse9Lib, kSse9Main, "42")
SYSV_BOTH_LIBS(vaArgMemoryStruct, "sysv_va_mem", kVaMemLibTrans, kVaMemLibGcc, kVaMemMain, "42")

// P2
SYSV_BOTH(nestedStructPass, "sysv_nest_pass", kNestLib, kNestMain, "42")
SYSV_BOTH(unionByValuePass, "sysv_union_pass", kUnionLib, kUnionMain, "42")
SYSV_BOTH(tinyThreeCharPass, "sysv_tiny3", kTinyLib, kTinyMain, "42")
SYSV_BOTH(fptrArg, "sysv_fptr_arg", kFptrArgLib, kFptrArgMain, "42")
SYSV_BOTH(fptrInStructPass, "sysv_fptr_s", kFptrStructLib, kFptrStructMain, "42")
SYSV_BOTH_LIBS(vaDefaultPromotions, "sysv_va_prom", kVaPromLibTrans, kVaPromLibGcc, kVaPromMain, "-304")

} // namespace
