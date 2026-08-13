#include "SysVAbiInteropHarness.h"

namespace {

using sysv_abi_interop::bothDirections;

constexpr const char* kBitFieldPackLib = R"prg(
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        int sum_pack(struct Pack p) {
            return p.a + p.b + p.c;
        }
    )prg";

constexpr const char* kBitFieldPackMain = R"prg(
        int printf(const char *, ...);
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        int sum_pack(struct Pack p);
        int main(void) {
            struct Pack p;
            p.a = 1;
            p.b = 2;
            p.c = 3;
            printf("%d", sum_pack(p));
            return 0;
        }
    )prg";

constexpr const char* kBitFieldMakeLib = R"prg(
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        struct Pack make_pack(void) {
            struct Pack p;
            p.a = 1;
            p.b = 2;
            p.c = 3;
            return p;
        }
    )prg";

constexpr const char* kBitFieldMakeMain = R"prg(
        int printf(const char *, ...);
        struct Pack {
            int a:4;
            int b:4;
            int c:8;
        };
        struct Pack make_pack(void);
        int main(void) {
            struct Pack p;
            p = make_pack();
            printf("%d %d %d", p.a, p.b, p.c);
            return 0;
        }
    )prg";

TEST(SysVAbi, bitFieldPack) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_bf_pack", kBitFieldPackLib, kBitFieldPackMain, "6"));
}

TEST(SysVAbi, bitFieldReturn) {
    ASSERT_NO_FATAL_FAILURE(bothDirections(
            "sysv_bf_ret", kBitFieldMakeLib, kBitFieldMakeMain, "1 2 3"));
}
} // namespace
