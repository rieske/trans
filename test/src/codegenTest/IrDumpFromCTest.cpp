#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CompileToIr.h"

namespace {

using namespace testing;

TEST(IrDumpFromC, fileScopeArithmetic) {
    EXPECT_THAT(compileToIr("int add(int a, int b) { return a + b; }\n"), StrEq(
            "PROC add\n"
            "\t$t0 := L$loc1_a + L$loc1_b\n"
            "\tRETURN $t0\n"
            "ENDPROC add\n"));
}

TEST(IrDumpFromC, ifElse) {
    EXPECT_THAT(compileToIr("int sel(int x) { if (x) return 1; return 0; }\n"), StrEq(
            "PROC sel\n"
            "\tCMP L$loc1_x, 0\n"
            "\tJE __L0\n"
            "\t$t0 := 1\n"
            "\tRETURN $t0\n"
            "__L0:\n"
            "\t$t1 := 0\n"
            "\tRETURN $t1\n"
            "ENDPROC sel\n"));
}

TEST(IrDumpFromC, call) {
    EXPECT_THAT(compileToIr("int g(int x); int f(int x) { return g(x); }\n"), StrEq(
            "PROC f\n"
            "\t$t0 := &g (function)\n"
            "\tPARAM L$loc1_x\n"
            "\tCALL g\n"
            "\tRETRIEVE $t1\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, structField) {
    EXPECT_THAT(compileToIr("struct S { int x; int y; }; int gety(struct S *p) { return p->y; }\n"),
            StrEq(
                    "PROC gety\n"
                    "\t$t0 := &(L$loc1_p->4)\n"
                    "\t$t1 := *$t0\n"
                    "\tRETURN $t1\n"
                    "ENDPROC gety\n"));
}

TEST(IrDumpFromC, vlaSizeofIsUseTimeProduct) {
    EXPECT_THAT(compileToIr("int vlasz(int n) { return (int)sizeof(int [n]); }\n"), StrEq(
            "PROC vlasz\n"
            "\t$t0 := L$loc1_n\n"
            "\t__t0 := 4\n"
            "\t$t0 := $t0 * __t0\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC vlasz\n"));
}

TEST(IrDumpFromC, gotoSkipsUnreachableLabel) {
    EXPECT_THAT(compileToIr("int f(void) { goto end; dead: return 1; end: return 0; }\n"), StrEq(
            "PROC f\n"
            "__L0:\n"
            "\t$t0 := 0\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, gotoKeepsUnreachableLabelAtO0) {
    const char* src = "int f(void) { goto end; dead: return 1; end: return 0; }\n";
    const std::string o0 = compileToIr(src, 0);
    EXPECT_THAT(o0, HasSubstr("GOTO"));
    EXPECT_THAT(o0, HasSubstr(":= 1"));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr(":= 1")));
}

TEST(IrDumpFromC, dumpsAreStableAcrossCalls) {
    const char* src = "int add(int a, int b) { return a + b; }\n";
    EXPECT_EQ(compileToIr(src), compileToIr(src));
}

} // namespace
