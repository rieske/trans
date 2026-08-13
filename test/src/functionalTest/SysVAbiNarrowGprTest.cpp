#include "SysVAbiInteropHarness.h"

namespace {

using sysv_abi_interop::Compiler;
using sysv_abi_interop::linkRunExpect;

// Callees that trust SysV 32/64-bit extension (no re-extend from dil/di).
constexpr const char* kNarrowGprLibGcc = R"prg(
        int take_uc32(unsigned char);
        int take_sc32(char);
        int take_us32(unsigned short);
        long take_ui64(unsigned);
        __asm__(
            ".text\n"
            ".globl take_uc32\n"
            "take_uc32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_sc32\n"
            "take_sc32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_us32\n"
            "take_us32:\n"
            "  movl %edi, %eax\n"
            "  ret\n"
            ".globl take_ui64\n"
            "take_ui64:\n"
            "  movq %rdi, %rax\n"
            "  ret\n"
        );
    )prg";

constexpr const char* kNarrowGprMainTrans = R"prg(
        int printf(const char *, ...);
        int take_uc32(unsigned char);
        int take_sc32(char);
        int take_us32(unsigned short);
        long take_ui64(unsigned);
        int main(void) {
            unsigned char uc[1];
            char sc[1];
            unsigned short us[2];
            unsigned ui;
            long wid;
            uc[0] = 255;
            sc[0] = -1;
            us[0] = 65535;
            us[1] = 1;
            ui = 0;
            ui = ui - 1;
            wid = take_ui64(ui);
            printf("%d %d %d %d", take_uc32(uc[0]), take_sc32(sc[0]), take_us32(us[0]),
                    (int)(wid > 0));
            return 0;
        }
    )prg";

constexpr const char* kNarrowGprLibTrans = R"prg(
        int take_uc(unsigned char c) {
            return c;
        }
        int take_sc(char c) {
            return c;
        }
        int take_us(unsigned short s) {
            return s;
        }
        unsigned take_ui(unsigned u) {
            return u;
        }
    )prg";

constexpr const char* kNarrowGprMainGcc = R"prg(
        int printf(const char *, ...);
        int take_uc(unsigned char);
        int take_sc(char);
        int take_us(unsigned short);
        unsigned take_ui(unsigned);
        int main(void) {
            printf("%d %d %d %d", take_uc(255), take_sc(-1), take_us(65535), (int)take_ui(4294967295u));
            return 0;
        }
    )prg";

TEST(SysVAbi, narrowGprExtend_transCallsGcc) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_narrow_gpr_tg", Compiler::Gcc, Compiler::Trans,
            kNarrowGprLibGcc, kNarrowGprMainTrans, "255 -1 65535 1"));
}

TEST(SysVAbi, narrowGprExtend_gccCallsTrans) {
    ASSERT_NO_FATAL_FAILURE(linkRunExpect(
            "sysv_narrow_gpr_gt", Compiler::Trans, Compiler::Gcc,
            kNarrowGprLibTrans, kNarrowGprMainGcc, "255 -1 65535 -1"));
}
} // namespace
