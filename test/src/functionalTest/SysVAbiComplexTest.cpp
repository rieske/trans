#include "SysVAbiHarness.h"

namespace {

using sysv_abi::Compiler;
using sysv_abi::linkRunExpect;

constexpr const char* kComplexFloatIdentLib = R"prg(
        _Complex float ident_cf(_Complex float x) {
            return x;
        }
    )prg";

constexpr const char* kComplexFloatIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float ident_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            z = ident_cf(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatMakeSplitLibGcc = R"prg(
        _Complex float make_cf(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            return z;
        }
        int split_cf(_Complex float z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexFloatMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex float make_cf(void);
        int split_cf(_Complex float);
        int main(void) {
            printf("%d", split_cf(make_cf()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatIdentLib, kComplexFloatIdentMainGcc, "1 2"));
}

TEST(SysVAbi, complexFloatPassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexFloatMakeSplitLibGcc, kComplexFloatMakeSplitMainTrans, "21"));
}

constexpr const char* kComplexDoubleIdentLib = R"prg(
        _Complex double ident_cd(_Complex double x) {
            return x;
        }
    )prg";

constexpr const char* kComplexDoubleIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double ident_cd(_Complex double);
        int main(void) {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 4.0;
            z = ident_cd(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleMakeSplitLibGcc = R"prg(
        _Complex double make_cd(void) {
            _Complex double z;
            __real__ z = 3.0;
            __imag__ z = 4.0;
            return z;
        }
        int split_cd(_Complex double z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexDoubleMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex double make_cd(void);
        int split_cd(_Complex double);
        int main(void) {
            printf("%d", split_cd(make_cd()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexDoubleIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleIdentLib, kComplexDoubleIdentMainGcc, "3 4"));
}

TEST(SysVAbi, complexDoublePassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexDoubleMakeSplitLibGcc, kComplexDoubleMakeSplitMainTrans, "43"));
}

constexpr const char* kComplexDoubleSecondLib = R"prg(
        _Complex double second_cd(_Complex double a, _Complex double b) {
            return b;
        }
    )prg";

constexpr const char* kComplexDoubleSecondMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double second_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 5.0;
            __imag__ b = 6.0;
            a = second_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexDoubleSecondArg_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_2nd_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleSecondLib, kComplexDoubleSecondMainGcc, "5 6"));
}

constexpr const char* kComplexLongDoubleIdentLib = R"prg(
        _Complex long double ident_cld(_Complex long double x) {
            return x;
        }
    )prg";

constexpr const char* kComplexLongDoubleIdentMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double ident_cld(_Complex long double);
        int main(void) {
            _Complex long double z;
            __real__ z = 7.0L;
            __imag__ z = 8.0L;
            z = ident_cld(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleMakeSplitLibGcc = R"prg(
        _Complex long double make_cld(void) {
            _Complex long double z;
            __real__ z = 7.0L;
            __imag__ z = 8.0L;
            return z;
        }
        int split_cld(_Complex long double z) {
            return (int)__real__ z + 10 * (int)__imag__ z;
        }
    )prg";

constexpr const char* kComplexLongDoubleMakeSplitMainTrans = R"prg(
        int printf(const char *, ...);
        _Complex long double make_cld(void);
        int split_cld(_Complex long double);
        int main(void) {
            printf("%d", split_cld(make_cld()));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexLongDoubleIdent_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_ident_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleIdentLib, kComplexLongDoubleIdentMainGcc, "7 8"));
}

constexpr const char* kComplexFloatAddLibTrans = R"prg(
        _Complex float add_cf(_Complex float a, _Complex float b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexFloatAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float add_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 3.0f;
            __imag__ b = 4.0f;
            a = add_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatFromRealLibTrans = R"prg(
        _Complex float from_f(float x) {
            return x;
        }
    )prg";

constexpr const char* kComplexFloatFromRealMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float from_f(float);
        int main(void) {
            _Complex float z;
            z = from_f(3.0f);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatToRealLibTrans = R"prg(
        float real_cf(_Complex float z) {
            return z;
        }
    )prg";

constexpr const char* kComplexFloatToRealMainGcc = R"prg(
        int printf(const char *, ...);
        float real_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            printf("%d", (int)real_cf(z));
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatNegLibTrans = R"prg(
        _Complex float neg_cf(_Complex float z) {
            return -z;
        }
    )prg";

constexpr const char* kComplexFloatNegMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float neg_cf(_Complex float);
        int main(void) {
            _Complex float z;
            __real__ z = 1.0f;
            __imag__ z = 2.0f;
            z = neg_cf(z);
            printf("%d %d", (int)__real__ z, (int)__imag__ z);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleAddLibTrans = R"prg(
        _Complex double add_cd(_Complex double a, _Complex double b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexDoubleAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double add_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 3.0;
            __imag__ b = 4.0;
            a = add_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleAddLibTrans = R"prg(
        _Complex long double add_cld(_Complex long double a, _Complex long double b) {
            return a + b;
        }
    )prg";

constexpr const char* kComplexLongDoubleAddMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double add_cld(_Complex long double, _Complex long double);
        int main(void) {
            _Complex long double a;
            _Complex long double b;
            __real__ a = 1.0L;
            __imag__ a = 2.0L;
            __real__ b = 3.0L;
            __imag__ b = 4.0L;
            a = add_cld(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatAddLibTrans, kComplexFloatAddMainGcc, "4 6"));
}

TEST(SysVAbi, complexFloatFromRealZerosImag_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_real_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatFromRealLibTrans, kComplexFloatFromRealMainGcc, "3 0"));
}

TEST(SysVAbi, complexFloatToRealDropsImag_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_drop_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatToRealLibTrans, kComplexFloatToRealMainGcc, "1"));
}

TEST(SysVAbi, complexFloatNeg_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_neg_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatNegLibTrans, kComplexFloatNegMainGcc, "-1 -2"));
}

TEST(SysVAbi, complexDoubleAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleAddLibTrans, kComplexDoubleAddMainGcc, "4 6"));
}

constexpr const char* kComplexFloatMulLibTrans = R"prg(
        _Complex float mul_cf(_Complex float a, _Complex float b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexFloatMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float mul_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 3.0f;
            __imag__ b = 4.0f;
            a = mul_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexFloatDivLibTrans = R"prg(
        _Complex float div_cf(_Complex float a, _Complex float b) {
            return a / b;
        }
    )prg";

constexpr const char* kComplexFloatDivMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex float div_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 0.0f;
            __imag__ a = 2.0f;
            __real__ b = 1.0f;
            __imag__ b = 1.0f;
            a = div_cf(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexDoubleMulLibTrans = R"prg(
        _Complex double mul_cd(_Complex double a, _Complex double b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexDoubleMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex double mul_cd(_Complex double, _Complex double);
        int main(void) {
            _Complex double a;
            _Complex double b;
            __real__ a = 1.0;
            __imag__ a = 2.0;
            __real__ b = 3.0;
            __imag__ b = 4.0;
            a = mul_cd(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

constexpr const char* kComplexLongDoubleMulLibTrans = R"prg(
        _Complex long double mul_cld(_Complex long double a, _Complex long double b) {
            return a * b;
        }
    )prg";

constexpr const char* kComplexLongDoubleMulMainGcc = R"prg(
        int printf(const char *, ...);
        _Complex long double mul_cld(_Complex long double, _Complex long double);
        int main(void) {
            _Complex long double a;
            _Complex long double b;
            __real__ a = 1.0L;
            __imag__ a = 2.0L;
            __real__ b = 3.0L;
            __imag__ b = 4.0L;
            a = mul_cld(a, b);
            printf("%d %d", (int)__real__ a, (int)__imag__ a);
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatMulLibTrans, kComplexFloatMulMainGcc, "-5 10"));
}

TEST(SysVAbi, complexFloatDiv_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_div_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatDivLibTrans, kComplexFloatDivMainGcc, "1 1"));
}

TEST(SysVAbi, complexDoubleMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cd_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexDoubleMulLibTrans, kComplexDoubleMulMainGcc, "-5 10"));
}

constexpr const char* kComplexFloatEqLibTrans = R"prg(
        int eq_cf(_Complex float a, _Complex float b) {
            return a == b;
        }
        int ne_cf(_Complex float a, _Complex float b) {
            return a != b;
        }
    )prg";

constexpr const char* kComplexFloatEqMainGcc = R"prg(
        int printf(const char *, ...);
        int eq_cf(_Complex float, _Complex float);
        int ne_cf(_Complex float, _Complex float);
        int main(void) {
            _Complex float a;
            _Complex float b;
            __real__ a = 1.0f;
            __imag__ a = 2.0f;
            __real__ b = 1.0f;
            __imag__ b = 2.0f;
            int same = eq_cf(a, b);
            __imag__ b = 3.0f;
            printf("%d %d", same, ne_cf(a, b));
            return 0;
        }
    )prg";

TEST(SysVAbi, complexFloatEq_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cf_eq_gt", Compiler::Trans, Compiler::Gcc,
            kComplexFloatEqLibTrans, kComplexFloatEqMainGcc, "1 1"));
}

TEST(SysVAbi, complexLongDoubleMul_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_mul_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleMulLibTrans, kComplexLongDoubleMulMainGcc, "-5 10"));
}

TEST(SysVAbi, complexLongDoubleAdd_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_add_gt", Compiler::Trans, Compiler::Gcc,
            kComplexLongDoubleAddLibTrans, kComplexLongDoubleAddMainGcc, "4 6"));
}

TEST(SysVAbi, complexLongDoublePassReturn_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_cld_pass_tg", Compiler::Gcc, Compiler::Trans,
            kComplexLongDoubleMakeSplitLibGcc, kComplexLongDoubleMakeSplitMainTrans, "87"));
}

} // namespace
