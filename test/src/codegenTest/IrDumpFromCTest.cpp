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
            " int f(int n) { return a(n); }\n";
    const std::string o1 = compileToIr(src, 1);
    EXPECT_THAT(o1, HasSubstr("PROC a\n"));
    EXPECT_THAT(o1, HasSubstr("PROC b\n"));
    const std::string a = procedureDump(o1, "a");
    const std::string b = procedureDump(o1, "b");
    EXPECT_TRUE(a.find("CALL a") != std::string::npos || a.find("CALL b") != std::string::npos
            || b.find("CALL a") != std::string::npos || b.find("CALL b") != std::string::npos);
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
            "\t$t0 := 1\n"
            "\tL$loc1_a := $t0\n"
            "\t$t1 := 2\n"
            "\tL$loc1_b := $t1\n"
            "\t$t2 := 3\n"
            "\tRETURN $t2\n"
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
            "\t__t0 := 1\n"
            "\tL$loc1_p := L$loc1_p + __t0*4 (ptr)\n"
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
