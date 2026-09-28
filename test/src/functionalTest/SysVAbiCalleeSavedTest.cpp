#include "SysVAbiInteropHarness.h"

// A gcc -O2 caller keeps a value in a callee-saved register across the call.
// The trans callee must return that register unchanged.

namespace {

constexpr const char* kPointerDiffLib =
        "int span(int *hi, int *lo) {\n"
        "  return (int)(hi - lo);\n"
        "}\n";

constexpr const char* kPointerDiffMain =
        "int printf(const char *, ...);\n"
        "int span(int *, int *);\n"
        "int main(int argc, char **argv) {\n"
        "  int a[8];\n"
        "  int keep = argc + 40;\n"
        "  int d = span(a + 5, a);\n"
        "  printf(\"%d %d\\n\", d, keep);\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kEarlyReturnLib =
        "int skewed(int *p) {\n"
        "  if (*p)\n"
        "    return 7;\n"
        "  int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;\n"
        "  return a + b + c + d + e + f + g + h + *p;\n"
        "}\n";

constexpr const char* kEarlyReturnMain =
        "int printf(const char *, ...);\n"
        "int skewed(int *);\n"
        "int main(int argc, char **argv) {\n"
        "  int flag = 1;\n"
        "  int keep = argc + 11;\n"
        "  int r = skewed(&flag);\n"
        "  printf(\"%d %d\\n\", r, keep);\n"
        "  return 0;\n"
        "}\n";

constexpr const char* kLaterReturnMain =
        "int printf(const char *, ...);\n"
        "int skewed(int *);\n"
        "int main(int argc, char **argv) {\n"
        "  int flag = 0;\n"
        "  int keep = argc + 11;\n"
        "  int r = skewed(&flag);\n"
        "  printf(\"%d %d\\n\", r, keep);\n"
        "  return 0;\n"
        "}\n";

SYSV_BOTH(PointerDiffPreservesCalleeSaved, "callee_ptr_diff", kPointerDiffLib, kPointerDiffMain,
        "5 41\n")

SYSV_BOTH(EarlyReturnPreservesCalleeSaved, "callee_early_ret", kEarlyReturnLib, kEarlyReturnMain,
        "7 12\n")

SYSV_BOTH(LaterReturnPreservesCalleeSaved, "callee_later_ret", kEarlyReturnLib, kLaterReturnMain,
        "36 12\n")

TEST(Compiler, earlyReturnEpilogueReloadsCalleeSaved) {
    SourceProgram program { std::string("int printf(const char *, ...);\n") + kEarlyReturnLib
                    + kEarlyReturnMain,
            { "-save-temps" } };
    program.compile();
    program.runAndExpect("7 12\n");
    const std::string assembly = program.readAssembly();
    const auto skewed = assembly.find("skewed:");
    ASSERT_NE(skewed, std::string::npos);
    const std::string body = assembly.substr(skewed);
    if (functionalTestOptFlag() == "-O0") {
        EXPECT_EQ(body.find("rbx"), std::string::npos);
        return;
    }
    const bool att = functionalTestDialectTag() == "att";
    const std::string pushRbx = att ? "pushq %rbx" : "push rbx";
    const std::string popRbx = att ? "popq %rbx" : "pop rbx";
    const auto push = body.find(pushRbx);
    const auto pop = body.find(popRbx);
    ASSERT_NE(push, std::string::npos);
    ASSERT_NE(pop, std::string::npos);
    EXPECT_LT(push, pop);
    EXPECT_EQ(body.find("__epi"), std::string::npos);
    EXPECT_NE(body.find("rbx", pop + popRbx.size()), std::string::npos);
}

} // namespace
