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
