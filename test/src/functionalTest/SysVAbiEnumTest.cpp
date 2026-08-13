#include "SysVAbiInteropHarness.h"

// Enum underlying width as ABI INTEGER: gcc↔trans must agree on size and value.

namespace {

// long-backed enum (value past UINT_MAX): 8-byte INTEGER pass.
constexpr const char* kLargeLib =
        "enum E { A = 0x100000000L };\n"
        "long take_e(enum E e) { return (long)e; }\n";
constexpr const char* kLargeMain =
        "int printf(const char *, ...);\n"
        "enum E { A = 0x100000000L };\n"
        "long take_e(enum E);\n"
        "int main(void) {\n"
        "  printf(\"%ld %d\", take_e(A), (int)sizeof(enum E));\n"
        "  return 0;\n"
        "}\n";

// unsigned int-backed enum (0x80000000): 4-byte INTEGER pass.
constexpr const char* kUintLib =
        "enum U { X = 0x80000000u };\n"
        "unsigned take_u(enum U u) { return (unsigned)u; }\n";
constexpr const char* kUintMain =
        "int printf(const char *, ...);\n"
        "enum U { X = 0x80000000u };\n"
        "unsigned take_u(enum U);\n"
        "int main(void) {\n"
        "  printf(\"%u %d\", take_u(X), (int)sizeof(enum U));\n"
        "  return 0;\n"
        "}\n";

SYSV_BOTH(enumLargeUnderlyingPass, "sysv_enum_large", kLargeLib, kLargeMain, "4294967296 8")
SYSV_BOTH(enumUnsignedIntUnderlyingPass, "sysv_enum_uint", kUintLib, kUintMain, "2147483648 4")

} // namespace
