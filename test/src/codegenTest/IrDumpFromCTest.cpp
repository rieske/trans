#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CompileToIr.h"

#include <string>
#include <string_view>

namespace {

using namespace testing;

std::string procedureDump(const std::string& dump, std::string_view name) {
    const std::string start = "PROC " + std::string(name) + "\n";
    const std::string end = "ENDPROC " + std::string(name) + "\n";
    const auto i = dump.find(start);
    if (i == std::string::npos) {
        return {};
    }
    const auto j = dump.find(end, i);
    if (j == std::string::npos || j < i) {
        return {};
    }
    return dump.substr(i, j - i);
}

int countSubstr(const std::string& text, std::string_view needle) {
    int n = 0;
    for (std::size_t i = 0; (i = text.find(needle, i)) != std::string::npos; ++i) {
        ++n;
    }
    return n;
}

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

TEST(IrDumpFromC, staticHelperAdd1StillCallsAtO0AndO1) {
    const char* src = "static int add1(int x) { return x + 1; } int f(int y) { return add1(y); }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC add1\n"
            "\t$t0 := 1\n"
            "\t$t1 := L$loc1_x + $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC add1\n"
            "PROC f\n"
            "\t$t2 := &add1 (function)\n"
            "\tPARAM L$loc2_y\n"
            "\tCALL add1\n"
            "\tRETRIEVE $t3\n"
            "\tRETURN $t3\n"
            "ENDPROC f\n"));
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, HasSubstr("PROC add1\n"));
    EXPECT_THAT(o1, HasSubstr("ENDPROC add1\n"));
    EXPECT_THAT(o1, Not(HasSubstr("CALL add1")));
    EXPECT_THAT(o1, Not(HasSubstr("PARAM")));
    const std::string f = procedureDump(o1, "f");
    EXPECT_THAT(f, Not(HasSubstr("GOTO")));
    EXPECT_THAT(f, HasSubstr("L$loc2_y"));
}

TEST(IrDumpFromC, add1OfConstantFoldsTo42AtO1) {
    const char* src = "static int add1(int x) { return x + 1; } int f(void) { return add1(41); }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("CALL add1"));
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, Not(HasSubstr("CALL")));
    EXPECT_THAT(o1, HasSubstr("42"));
}

TEST(IrDumpFromC, twoNextCallsShareStaticLocal) {
    const char* src = "static int next(void) { static int n; return ++n; }"
            " int f(void) { return next() + next(); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, Not(HasSubstr("CALL next")));
    EXPECT_THAT(countSubstr(procedureDump(o1, "f"), "INC L$st1_n"), Eq(2));
}

TEST(IrDumpFromC, twoAdd1SitesFoldIndependently) {
    const char* src = "static int add1(int x) { return x + 1; }"
            " int f(void) { return add1(1) + add1(2); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, Not(HasSubstr("CALL add1")));
    EXPECT_THAT(o1, AnyOf(HasSubstr(":= 5"), HasSubstr("RETURN 5")));
}

TEST(IrDumpFromC, selfRecursionKeepsCall) {
    const char* src = "static int fact(int n) { if (n <= 1) return 1; return n * fact(n - 1); }"
            " int f(int n) { return fact(n); }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("CALL fact"));
    EXPECT_THAT(procedureDump(compileToIr(src, 1), "fact"), HasSubstr("CALL fact"));
}

TEST(IrDumpFromC, mutualRecursionKeepsBackEdge) {
    const char* src = "static int b(int n);"
            " static int a(int n) { if (n) return b(n - 1); return 0; }"
            " static int b(int n) { if (n) return a(n - 1); return 1; }"
            " static int add1(int x) { return x + 1; }"
            " int f(int n) { return a(n) + add1(1); }\n";
    const std::string o0 = compileToIr(src, 0);
    EXPECT_THAT(procedureDump(o0, "a"), HasSubstr("CALL b"));
    EXPECT_THAT(procedureDump(o0, "f"), HasSubstr("CALL add1"));
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(procedureDump(o1, "a"), HasSubstr("CALL a"));
    EXPECT_THAT(procedureDump(o1, "b"), HasSubstr("CALL a"));
    EXPECT_THAT(procedureDump(o1, "f"), Not(HasSubstr("CALL add1")));
}

TEST(IrDumpFromC, indirectCallKeepsStarAtO1) {
    const char* src = "static int add1(int x) { return x + 1; }"
            " int f(int y) { int (*fp)(int) = add1; return add1(y) + fp(y); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, HasSubstr("PROC add1\n"));
    const std::string f = procedureDump(o1, "f");
    EXPECT_THAT(f, HasSubstr("CALL *"));
    EXPECT_THAT(f, Not(HasSubstr("CALL add1")));
}

TEST(IrDumpFromC, variadicCalleeKeepsCallAtO1) {
    const char* src = "static int sum(int n, ...) { return n; } int f(void) { return sum(1); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, HasSubstr("PROC sum variadic\n"));
    EXPECT_THAT(procedureDump(o1, "f"), HasSubstr("CALL sum"));
}

TEST(IrDumpFromC, oversizeCalleeKeepsCallAtO1) {
    std::string src = "static int fat(int x) {\n";
    for (int i = 0; i < 32; ++i) {
        src += "++x;\n";
    }
    src += "return x;\n} int f(int y) { return fat(y); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(countSubstr(procedureDump(o1, "fat"), "INC "), Eq(32));
    EXPECT_THAT(procedureDump(o1, "f"), HasSubstr("CALL fat"));
}

TEST(IrDumpFromC, call) {
    EXPECT_THAT(compileToIr("int g(int x); int f(int x) { return g(x); }\n"), StrEq(
            "PROC f\n"
            "\tPARAM L$loc1_x\n"
            "\tCALL g\n"
            "\tRETRIEVE $t0\n"
            "\tRETURN $t0\n"
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

TEST(IrDumpFromC, licmHoistsAddWhenLoopMayNotRun) {
    const char* src = "int f(int n) { int i; int s = 7; for (i = 0; i < n; i++) s += n + 1; return s; }\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto add0 = o0.find(" + ");
    const auto lab0 = o0.find("\n__L");
    const auto add1 = o1.find(" + ");
    const auto lab1 = o1.find("\n__L");
    const auto acc = o1.find("L$loc1_s := L$loc1_s + ");
    ASSERT_THAT(add0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(add1, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    ASSERT_THAT(acc, Ne(std::string::npos));
    EXPECT_THAT(add0, Gt(lab0));
    EXPECT_THAT(add1, Lt(lab1));
    EXPECT_THAT(acc, Gt(lab1));
}

TEST(IrDumpFromC, licmHoistsNestedInvariantToOuterPreheader) {
    const char* src = "int f(int n) {\n"
            "  int i, j, s = 0;\n"
            "  for (i = 0; i < n; i++)\n"
            "    for (j = 0; j < n; j++)\n"
            "      s += n + 1;\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto add0 = o0.find("L$loc1_n + ");
    const auto lab0 = o0.find("\n__L");
    const auto add1 = o1.find("L$loc1_n + ");
    const auto lab1 = o1.find("\n__L");
    const auto acc = o1.find("L$loc1_s := L$loc1_s + ");
    ASSERT_THAT(add0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(add1, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    ASSERT_THAT(acc, Ne(std::string::npos));
    EXPECT_THAT(add0, Gt(lab0));
    EXPECT_THAT(add1, Lt(lab1));
    EXPECT_THAT(acc, Gt(lab1));
    EXPECT_THAT(countSubstr(o1, "L$loc1_n + "), Eq(1));
}

TEST(IrDumpFromC, licmHoistsInvariantAddInForAtO1) {
    const char* src = "int f(int n) { int i; int s = 0; for (i = 0; i < n; i++) s += n + 1; return s; }\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o0.find(" + "), Gt(o0.find("\n__L")));
    EXPECT_THAT(o1.find(" + "), Lt(o1.find("\n__L")));
}

TEST(IrDumpFromC, deadNamedLocalIsRemoved) {
    const char* src = "int f(int a) { int x; x = a; return 0; }\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o0, HasSubstr("L$loc1_x"));
    EXPECT_THAT(o1, Not(HasSubstr("L$loc1_x")));
    EXPECT_THAT(o1, HasSubstr("RETURN"));
}

TEST(IrDumpFromC, deadMulIsNotStrengthReduced) {
    const char* src = "int f(int n, int k) {\n"
            "  int i;\n"
            "  for (i = 0; i < n; i++)\n"
            "    (void)(i * k);\n"
            "  return 0;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o0, HasSubstr(" * "));
    EXPECT_THAT(o1, Not(HasSubstr(" * ")));
    EXPECT_THAT(o1, Not(HasSubstr("$sr")));
}

TEST(IrDumpFromC, foldHoistedConstantAdd) {
    const char* src = "int f(void) {\n"
            "  int i; int s = 0; int n = 3;\n"
            "  for (i = 0; i < n; i++) s += n + 1;\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o0.find(" + "), Gt(o0.find("\n__L")));
    EXPECT_THAT(o1, HasSubstr(":= 4"));
    EXPECT_THAT(o1, Not(HasSubstr("L$loc1_n + ")));
}

TEST(IrDumpFromC, strengthReduceRewritesMulWhenLoopMayNotRun) {
    const char* src = "int f(int n, int k) {\n"
            "  int i, s = 7;\n"
            "  for (i = 1; i < n; i++) s += i * k;\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto mul0 = o0.find(" * ");
    const auto lab0 = o0.find("\n__L");
    const auto folded = o1.find("$sr0 := L$loc1_k");
    const auto lab1 = o1.find("\n__L");
    const auto acc = o1.find("L$loc1_s := L$loc1_s + ");
    ASSERT_THAT(mul0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(folded, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    ASSERT_THAT(acc, Ne(std::string::npos));
    EXPECT_THAT(mul0, Gt(lab0));
    EXPECT_THAT(folded, Lt(lab1));
    EXPECT_THAT(acc, Gt(lab1));
    EXPECT_THAT(o1, Not(HasSubstr(" * ")));
    EXPECT_THAT(o1, HasSubstr("INC L$loc1_i\n\t$sr0 := $sr0 + L$loc1_k\n"));
}

TEST(IrDumpFromC, strengthReduceKeepsIndexInsideLoopWhenBaseSlides) {
    const char* src = "static int slide(int **p) { *p = *p + 1; return 0; }\n"
            "int f(int *a, int n, int k) {\n"
            "  long i;\n"
            "  int s = 0;\n"
            "  for (i = 0; i < n; i++) s += i * k;\n"
            "  for (i = 0; i < n; i++) { s += a[i]; slide(&a); }\n"
            "  return s;\n"
            "}\n";
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto sr = o1.find("$sr0 := 0");
    const auto lab = o1.find("\n__L");
    const auto stride = o1.find("stride=4");
    ASSERT_THAT(sr, Ne(std::string::npos));
    ASSERT_THAT(lab, Ne(std::string::npos));
    ASSERT_THAT(stride, Ne(std::string::npos));
    EXPECT_THAT(sr, Lt(lab));
    EXPECT_THAT(stride, Gt(lab));
    EXPECT_THAT(o1, Not(HasSubstr("$sr0 := &")));
    EXPECT_THAT(o1, HasSubstr("$sr0 := $sr0 + "));
}

TEST(IrDumpFromC, strengthReduceMovesMulOfIvBeforeLoop) {
    const char* src = "int f(int n, int k) {\n"
            "  int i, s = 0;\n"
            "  for (i = 0; i < n; i++) s += i * k;\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto mul0 = o0.find(" * ");
    const auto lab0 = o0.find("\n__L");
    const auto folded = o1.find("$sr0 := 0");
    const auto lab1 = o1.find("\n__L");
    ASSERT_THAT(mul0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(folded, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    EXPECT_THAT(mul0, Gt(lab0));
    EXPECT_THAT(folded, Lt(lab1));
    EXPECT_THAT(o1, Not(HasSubstr(" * ")));
    EXPECT_THAT(o1, HasSubstr("INC L$loc1_i\n\t$sr0 := $sr0 + L$loc1_k\n"));
}

TEST(IrDumpFromC, strengthReduceRewritesIndexWhenLoopMayNotRun) {
    const char* src = "int f(int *a, int n) {\n"
            "  long i;\n"
            "  int s = 7;\n"
            "  for (i = 1; i < n; i++) s += a[i];\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto stride0 = o0.find("stride=4");
    const auto lab0 = o0.find("\n__L");
    const auto stride1 = o1.find("$sr0 := &L$loc1_a[L$loc1_i] stride=4 (ptr)");
    const auto lab1 = o1.find("\n__L");
    const auto acc = o1.find("L$loc1_s := L$loc1_s + ");
    ASSERT_THAT(stride0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(stride1, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    ASSERT_THAT(acc, Ne(std::string::npos));
    EXPECT_THAT(stride0, Gt(lab0));
    EXPECT_THAT(stride1, Lt(lab1));
    EXPECT_THAT(acc, Gt(lab1));
}

TEST(IrDumpFromC, strengthReduceStepsIndexedPointer) {
    const char* src = "int f(int *a, int n) {\n"
            "  long i;\n"
            "  int s = 0;\n"
            "  for (i = 0; i < n; i++) s += a[i];\n"
            "  return s;\n"
            "}\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    const auto stride0 = o0.find("stride=4");
    const auto lab0 = o0.find("\n__L");
    const auto stride1 = o1.find("$sr0 := &L$loc1_a[L$loc1_i] stride=4 (ptr)");
    const auto lab1 = o1.find("\n__L");
    ASSERT_THAT(stride0, Ne(std::string::npos));
    ASSERT_THAT(lab0, Ne(std::string::npos));
    ASSERT_THAT(stride1, Ne(std::string::npos));
    ASSERT_THAT(lab1, Ne(std::string::npos));
    EXPECT_THAT(stride0, Gt(lab0));
    EXPECT_THAT(stride1, Lt(lab1));
    EXPECT_THAT(countSubstr(o1, "stride="), Eq(1));
    EXPECT_THAT(o1, HasSubstr("INC L$loc1_i\n\t$sr0 := $sr0 + 4 (ptr)\n"));
}

TEST(IrDumpFromC, strengthReducePrintsNegativePointerStep) {
    const char* src = "int f(int *a, int n) {\n"
            "  long i;\n"
            "  int s = 0;\n"
            "  for (i = n - 1; i >= 0; i--) s += a[i];\n"
            "  return s;\n"
            "}\n";
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o1, HasSubstr("$sr0 := $sr0 - 4 (ptr)\n"));
}

TEST(IrDumpFromC, licmDoesNotInsertPreheaderWhenNothingHoists) {
    const char* src = "int f(int n) { while (n) n--; return n; }\n";
    const std::string o0 = procedureDump(compileToIr(src, 0), "f");
    const std::string o1 = procedureDump(compileToIr(src, 1), "f");
    EXPECT_THAT(o1, HasSubstr("PROC f\n__L0:\n\tCMP "));
    EXPECT_THAT(countSubstr(o1, "\n__L"), Eq(2));
    EXPECT_THAT(countSubstr(o1, "\n__L"), Eq(countSubstr(o0, "\n__L")));
}

TEST(IrDumpFromC, vlaSizeofIsUseTimeProduct) {
    EXPECT_THAT(compileToIr("int vlasz(int n) { return (int)sizeof(int [n]); }\n"), StrEq(
            "PROC vlasz\n"
            "\t$t0 := L$loc1_n\n"
            "\t__t0 := 4\n"
            "\t$t0 := $t0 * __t0\n"
            "\tRETURN $t0\n"
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

TEST(IrDumpFromC, foldsLocalIntegerAddAtO1) {
    const char* src = "int f(void) { int a = 1; int b = 2; return a + b; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := 1\n"
            "\tL$loc1_a := $t0\n"
            "\t$t1 := 2\n"
            "\tL$loc1_b := $t1\n"
            "\t$t2 := L$loc1_a + L$loc1_b\n"
            "\tRETURN $t2\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := 3\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, foldsAddZeroAtO1) {
    const char* src = "int f(int a) { return a + 0; }\n";
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_a\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, foldsAndSelfAtO1) {
    const char* src = "int f(int a) { return a & a; }\n";
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_a\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, dumpsAreStableAcrossCalls) {
    const char* src = "int add(int a, int b) { return a + b; }\n";
    EXPECT_EQ(compileToIr(src), compileToIr(src));
}

TEST(IrDumpFromC, ifZeroDropsDeadArmAtO1) {
    const char* src = "int f(void) { if (0) return 1; return 0; }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("JE"));
    EXPECT_THAT(compileToIr(src, 0), HasSubstr(":= 1"));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr("JE")));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr(":= 1")));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr(":= 0"));
}

TEST(IrDumpFromC, ifOneDropsDeadArmAtO1) {
    const char* src = "int f(void) { if (1) return 1; return 0; }\n";
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr("JE")));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr(":= 0")));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr(":= 1"));
}

TEST(IrDumpFromC, ifConstRelDropsDeadArmAtO1) {
    const char* src = "int f(void) { if (1 < 2) return 1; return 0; }\n";
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr("CMP")));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr(":= 0")));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr(":= 1"));
}

TEST(IrDumpFromC, unusedIntegerAddDropsAtO1) {
    const char* src = "int f(int x) { x + 1; return 0; }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("+"));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr("+")));
}

TEST(IrDumpFromC, starAssignStoresThroughPointerAtO0AndO1) {
    const char* src = "void f(int *p) { *p = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_p\n"
            "\t$t1 := 9\n"
            "\t$t0 := $t1\n"
            "\t*L$loc1_p := $t0\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := 9\n"
            "\t*L$loc1_p := $t0\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starStarAssignStoresThroughInnerLoadAtO0AndO1) {
    const char* src = "void f(int **pp) { **pp = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_pp\n"
            "\t$t1 := *$t0\n"
            "\t$t2 := 9\n"
            "\t$t1 := $t2\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_pp\n"
            "\t$t1 := 9\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starStarStarAssignStoresThroughMiddleLoad) {
    const char* src = "void f(int ***ppp) { ***ppp = 8; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_ppp\n"
            "\t$t1 := *$t0\n"
            "\t$t2 := *$t1\n"
            "\t$t3 := 8\n"
            "\t$t2 := $t3\n"
            "\t*$t1 := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_ppp\n"
            "\t$t1 := *$t0\n"
            "\t$t2 := 8\n"
            "\t*$t1 := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starPostfixAssignStoresThroughSavedPointer) {
    const char* src = "void f(int *p) { *p++ = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t__t0 := 1\n"
            "\tL$loc1_p := L$loc1_p + __t0*4 (ptr)\n"
            "\t$t1 := *$t0\n"
            "\t$t2 := 9\n"
            "\t$t1 := $t2\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t$t1 := 9\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starPrefixAssignStoresThroughIncrementedPointer) {
    const char* src = "void f(int *p) { *++p = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("*L$loc1_p :="));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr("*L$loc1_p :="));
}

TEST(IrDumpFromC, starCastAssignStoresThroughCastTemp) {
    const char* src = "void f(int *p) { *(int *)p = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t$t1 := *$t0\n"
            "\t$t2 := 9\n"
            "\t$t1 := $t2\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t$t1 := 9\n"
            "\t*$t0 := $t1\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starOffsetAssignStoresThroughPointerOffset) {
    const char* src = "void f(int *p) { *(p + 1) = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := 1\n"
            "\t$t1 := L$loc1_p + $t0*4 (ptr)\n"
            "\t$t2 := *$t1\n"
            "\t$t3 := 9\n"
            "\t$t2 := $t3\n"
            "\t*$t1 := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := 1\n"
            "\t$t1 := L$loc1_p + $t0*4 (ptr)\n"
            "\t$t2 := 9\n"
            "\t*$t1 := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, parenStarMemberAssignUsesPointerAsFieldBase) {
    const char* src = "struct S { int x; int y; }; void f(struct S *p) { (*p).y = 9; }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("&(L$loc1_p->4)"));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr("&(L$loc1_p->4)"));
    EXPECT_THAT(compileToIr(src, 0), Not(HasSubstr("&($t")));
}

TEST(IrDumpFromC, parenStarPostfixMemberAssignUsesSavedPointerAsFieldBase) {
    const char* src = "struct S { int x; int y; }; void f(struct S *p) { (*p++).y = 9; }\n";
    const std::string o0 = compileToIr(src, 0);
    EXPECT_THAT(o0, HasSubstr("$t0 := L$loc1_p"));
    EXPECT_THAT(o0, HasSubstr("&($t0->4)"));
    EXPECT_THAT(o0, Not(HasSubstr("*L$loc1_p :=")));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr("&($t0->4)"));
}

TEST(IrDumpFromC, parenStarBitFieldAssignUsesPointerAsFieldBase) {
    const char* src = "struct S { int bf : 3; int y; }; void f(struct S *p) { (*p).bf = 1; }\n";
    EXPECT_THAT(compileToIr(src, 0), HasSubstr("&(L$loc1_p->"));
    EXPECT_THAT(compileToIr(src, 0), Not(HasSubstr("&($t")));
}

TEST(IrDumpFromC, arrowAndIndexAssignStayOnDefinedAddress) {
    EXPECT_THAT(compileToIr("struct S { int x; int y; }; void f(struct S *p) { p->y = 9; }\n", 0),
            HasSubstr("&(L$loc1_p->4)"));
    EXPECT_THAT(compileToIr("void f(void) { int a[4]; a[1] = 9; }\n", 0),
            HasSubstr("&L$loc1_a["));
    EXPECT_THAT(compileToIr("struct S { int x; int y; }; void f(struct S *p) { p->y = 9; }\n", 1),
            StrEq(
                    "PROC f\n"
                    "\t$t0 := &(L$loc1_p->4)\n"
                    "\t$t1 := 9\n"
                    "\t*$t0 := $t1\n"
                    "\tRETURN\n"
                    "ENDPROC f\n"));
    EXPECT_THAT(compileToIr("void f(void) { int a[4]; a[1] = 9; }\n", 1), StrEq(
            "PROC f\n"
            "\t$t0 := 1\n"
            "\t$t1 := &L$loc1_a[$t0] stride=4 (array)\n"
            "\t$t2 := 9\n"
            "\t*$t1 := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starRmwKeepsTheLoadAtO1) {
    EXPECT_THAT(compileToIr("void f(int *p) { ++*p; }\n", 1), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_p\n"
            "\tINC $t0\n"
            "\t*L$loc1_p := $t0\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, starAssignFromStarKeepsOneLoadAtO1) {
    EXPECT_THAT(compileToIr("void f(int *p) { *p = *p + 1; }\n", 1), StrEq(
            "PROC f\n"
            "\t$t0 := *L$loc1_p\n"
            "\t$t1 := 1\n"
            "\t$t2 := $t0 + $t1\n"
            "\t*L$loc1_p := $t2\n"
            "\tRETURN\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarDropsSelfCopyAtO0) {
    const char* src = "int *f(int *p) { return &*p; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarRowDoesNotAddressThePointerTemp) {
    const char* src = "int *f(int a[2][3], int i) { return &*a[i]; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := &L$loc1_a[L$loc1_i] stride=12 (ptr)\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := &L$loc1_a[L$loc1_i] stride=12 (ptr)\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarMemberArrayDoesNotAddressThePointerTemp) {
    const char* src = "struct S { int arr[4]; }; int *f(struct S *s) { return &*s->arr; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := &(L$loc1_s->0)\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := &(L$loc1_s->0)\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarStringDoesNotAddressThePointerTemp) {
    const char* src = "const char *f(void) { return &*\"xy\"; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := &L$str0\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := &L$str0\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarStarPtrToArrayDoesNotAddressThePointerTemp) {
    const char* src = "int *f(int (*p)[3]) { return &**p; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\tRETURN $t0\n"
            "ENDPROC f\n"));
}

TEST(IrDumpFromC, addressOfStarPostfixDoesNotUndoIncrement) {
    const char* src = "int *f(int *p) { return &*p++; }\n";
    EXPECT_THAT(compileToIr(src, 0), StrEq(
            "PROC f\n"
            "\t$t0 := L$loc1_p\n"
            "\t__t0 := 1\n"
            "\tL$loc1_p := L$loc1_p + __t0*4 (ptr)\n"
            "\t$t1 := $t0\n"
            "\tRETURN $t1\n"
            "ENDPROC f\n"));
    EXPECT_THAT(compileToIr(src, 1), HasSubstr("$t0 := L$loc1_p"));
    EXPECT_THAT(compileToIr(src, 1), Not(HasSubstr("L$loc1_p := $t0")));
}

TEST(IrDumpFromC, voidValuesAreNeverMaterialized) {
    EXPECT_THAT(compileToIr("void v(void){}\nint c = 1;\n"
                            "int main(void){ (void)v(); c ? v() : v(); return 0; }\n", 0), StrEq(
            "PROC v\n"
            "\tRETURN\n"
            "ENDPROC v\n"
            "PROC main\n"
            "\t$t0 := &v (function)\n"
            "\tCALL v\n"
            "\tCMP c, 0\n"
            "\tJE __L0\n"
            "\t$t1 := &v (function)\n"
            "\tCALL v\n"
            "\tGOTO __L1\n"
            "__L0:\n"
            "\t$t2 := &v (function)\n"
            "\tCALL v\n"
            "__L1:\n"
            "\t$t3 := 0\n"
            "\tRETURN $t3\n"
            "ENDPROC main\n"));
}

} // namespace
