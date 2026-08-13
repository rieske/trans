#include "TestFixtures.h"

namespace {

TEST(Compiler, leftoverPragmaDoesNotBreakParse) {
    SourceProgram program{R"prg(#include <stdio.h>
        #pragma once
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, c99BoolSpellingIsOneByte) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            _Bool b;
            b = 1;
            printf("%d %d", (int)sizeof(_Bool), (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, boolKeywordIsOneByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool b;
            b = 1;
            printf("%d %d", (int)sizeof(bool), (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, trueAndFalseAreBoolConstants) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool t;
            bool f;
            t = true;
            f = false;
            printf("%d %d %d", (int)t, (int)f, (int)sizeof(true));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 1");
}

TEST(Compiler, genericTrueIsBoolNotUnsignedChar) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic(true, bool: 1, unsigned char: 2, default: 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, assignToBoolConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool b;
            bool z;
            b = 2;
            z = 0;
            printf("%d %d", (int)b, (int)z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, initBoolConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool b = 2;
            printf("%d", (int)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, returnBoolConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool asBool(int n) {
            return n;
        }
        int main() {
            printf("%d %d", (int)asBool(2), (int)asBool(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, boolArgConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int show(bool b) {
            return (int)b;
        }
        int main() {
            printf("%d %d", show(2), show(0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, globalBoolInitConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool g = 2;
        bool z = 0;
        bool braced = { 5 };
        int main() {
            printf("%d %d %d", (int)g, (int)z, (int)braced);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 1");
}

TEST(Compiler, pointerToBoolConvertsNonNullToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            int *p;
            int *n;
            bool t;
            bool f;
            p = &x;
            n = 0;
            t = p;
            f = n;
            printf("%d %d", (int)t, (int)f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, floatToBoolConvertsNonzeroToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool t;
            bool f;
            t = 1.5;
            f = 0.0;
            printf("%d %d", (int)t, (int)f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, localBoolBraceAndArrayInitConvertToOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool b = { 2 };
            bool a[2] = { 2, 0 };
            printf("%d %d %d", (int)b, (int)a[0], (int)a[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0");
}

TEST(Compiler, skipPrefixImplGitShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static inline bool skip_prefix_impl(const char *str, const char *prefix,
                const char **out) {
            do {
                if (!*prefix) {
                    *out = str;
                    return true;
                }
            } while (*str++ == *prefix++);
            return false;
        }
        int main() {
            const char *out;
            printf("%d %d", skip_prefix_impl("abc", "ab", &out), *out == 'c');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, int128IsSixteenBytes) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", (int)sizeof(__int128));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}

TEST(Compiler, signedAndUnsignedInt128AreSixteenBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef __signed__ __int128 __s128;
        typedef unsigned __int128 __u128;
        int main() {
            printf("%d %d %d", (int)sizeof(__s128), (int)sizeof(__u128), (int)sizeof(__uint128_t));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16 16 16");
}

TEST(Compiler, float64TypedefStandInIsDouble) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", (int)sizeof(_Float64));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, inlineFunctionIsAccepted) {
    SourceProgram program{R"prg(#include <stdio.h>
        static inline int add1(int x) {
            return x + 1;
        }
        int main() {
            printf("%d", add1(41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, noreturnFunctionSpecifierIsAccepted) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        noreturn void stop(void);
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, c11NoreturnSpellingIsNoreturn) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        _Noreturn void OPENSSL_die(const char *assertion, const char *file, int line);
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, noreturnMixesWithStaticInline) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static inline noreturn void stop(void);
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, restrictPointerParamIsAccepted) {
    SourceProgram program{R"prg(#include <stdio.h>
        int load(const int * restrict p) {
            return *p;
        }
        int main() {
            int x;
            x = 7;
            printf("%d", load(&x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, restrictInUnsizedArrayParam) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int a[restrict]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 8;
            printf("%d", load(v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, restrictInSizedArrayParam) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int a[restrict 2]) {
            return a[1];
        }
        int main() {
            int v[2];
            v[0] = 1;
            v[1] = 9;
            printf("%d", load(v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, restrictInAbstractArrayParam) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int [restrict]);
        int load(int a[restrict]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 6;
            printf("%d", load(v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, restrictInAbstractSizedArrayParam) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int [restrict 2]);
        int load(int a[restrict 2]) {
            return a[1];
        }
        int main() {
            int v[2];
            v[0] = 1;
            v[1] = 4;
            printf("%d", load(v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, gnuRestrictInArrayParam) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int a[__restrict]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 3;
            printf("%d", load(v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, restrictInArrayParamWithPriorParamSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int load(int n, int a[restrict n]) {
            return a[0];
        }
        int main() {
            int v[1];
            v[0] = 2;
            printf("%d", load(1, v));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, funcNameIsCurrentFunction) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%s", __func__);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("main");
}

TEST(Compiler, attributeAfterDeclaratorIsIgnored) {
    SourceProgram program{R"prg(#include <stdio.h>
        int x __attribute__((unused));
        int main() {
            x = 3;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, adjacentStringsConcatenate) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%s", "ab" "cd");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("abcd");
}

TEST(Compiler, wideStringPrefixIsPlainString) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%s", L"ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, statementAsmIsNullStatement) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            asm volatile ("nop");
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, statementAsmGnuVolatileAndExtensionIsNullStatement) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            asm __volatile__ ("nop");
            asm __extension__ volatile ("nop");
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, asmBareSpellingOnDeclarator) {
    SourceProgram program{R"prg(#include <stdio.h>
        int y __asm("x");
        int main() {
            y = 3;
            printf("%d", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, adjacentStringsConcatenateAcrossExtension) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%s", "ab" __extension__ "cd");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("abcd");
}

TEST(Compiler, funcNameOutsideFunctionIsError) {
    SourceProgram program{R"prg(
        const char *p = __func__;
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("__func__ used outside a function");
}

TEST(Compiler, gnuStatementExprYieldsLastExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            x = ({ int y; y = 1; y; });
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, gnuStatementExprExtensionAndCommaVoidAssertShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = 1;
            ((void) sizeof(n), __extension__ ({ if (n) ; else ; }));
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, gnuStatementExprAsIfConditionAndCallArg) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int id(int n) {
            return n;
        }
        int main() {
            int x;
            x = 0;
            if (({ 1; })) {
                x = id(({ int y; y = 4; y; }));
            }
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, gnuStatementExprNested) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", ({ ({ 2; }); }));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, gnuStatementExprOuterTypedefIsVisibleInside) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef int T;
        int main() {
            T x;
            x = ({ T y; y = 3; y; });
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, gnuStatementExprOuterStructTagIsVisibleInside) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Pair {
            int a;
            int b;
        };
        int main() {
            struct Pair q;
            q.a = 11;
            q.b = 22;
            printf("%d", ({ struct Pair p; p = q; p.a + p.b; }));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("33");
}

TEST(Compiler, gnuStatementExprShadowDoesNotEscape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef int T;
        int main() {
            int x;
            x = ({ int T; T = 7; T; });
            T y;
            y = 1;
            printf("%d %d", x, y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 1");
}

TEST(Compiler, gnuStatementExprAsInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a = ({ int y; y = 5; y; });
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, gnuStatementExprDeclarationOnlyIsVoidStatement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            ({ int y; });
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, gnuStatementExprMissingCloseParenIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x = ({ 1; };
            return x;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("parsing failed with syntax errors");
}

TEST(Compiler, gnuStatementExprUnclosedIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x = ({ 1;
            return x;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("parsing failed with syntax errors");
}

TEST(Compiler, arrayParamInDeclaratorIsNotStatementExpr) {
    SourceProgram program{R"prg(
        int f({);
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("unexpected token: {");
}

TEST(Compiler, isoStdRejectsGnuStatementExpression) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = ({ 1; });
            return x;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, isoStdRejectsAttribute) {
    SourceProgram program{R"prg(
        int main() {
            int x __attribute__((unused));
            x = 1;
            return x;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, isoStdRejectsBuiltinVaArg) {
    SourceProgram program{R"prg(
        int f(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            int x = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return x;
        }
        int main() {
            return f(1, 2);
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, isoStdRejectsGnuInlineSpelling) {
    SourceProgram program{R"prg(
        __inline int f(void) {
            return 1;
        }
        int main() {
            return f();
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token");
}

TEST(Compiler, isoStdRejectsExtension) {
    SourceProgram program{R"prg(
        int main() {
            __extension__ int x;
            x = 1;
            return x;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token: int");
}

TEST(Compiler, isoStdRejectsAsm) {
    SourceProgram program{R"prg(
        int main() {
            asm("nop");
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `asm` is not defined");
}

TEST(Compiler, isoStdRejectsAsmUnderscores) {
    SourceProgram program{R"prg(
        int main() {
            __asm__("nop");
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__asm__` is not defined");
}

TEST(Compiler, isoStdRejectsAsmSingleUnderscore) {
    SourceProgram program{R"prg(
        int main() {
            __asm("nop");
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__asm` is not defined");
}

TEST(Compiler, isoStdRejectsGnuRestrict) {
    SourceProgram program{R"prg(
        int main() {
            int * __restrict p;
            int x;
            p = &x;
            return *p;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token: p");
}

TEST(Compiler, isoStdRejectsGnuConst) {
    SourceProgram program{R"prg(
        int main() {
            __const int x;
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token: int");
}

TEST(Compiler, isoStdRejectsGnuSigned) {
    SourceProgram program{R"prg(
        int main() {
            __signed int x;
            x = 1;
            return x;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token: int");
}

TEST(Compiler, isoStdRejectsBuiltinVaList) {
    SourceProgram program{R"prg(
        int main() {
            __builtin_va_list ap;
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("unexpected token: ap");
}

TEST(Compiler, isoStdRejectsGnuReal) {
    SourceProgram program{R"prg(
        int main() {
            _Complex double z;
            z = 1.0;
            return (int)__real__(z);
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__real__` is not defined");
}

TEST(Compiler, isoStdRejectsGnuImag) {
    SourceProgram program{R"prg(
        int main() {
            _Complex double z;
            z = 1.0;
            return (int)__imag__(z);
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__imag__` is not defined");
}

TEST(Compiler, isoStdRejectsImaginarySuffix) {
    SourceProgram program{R"prg(
        int main() {
            _Complex double z;
            z = 1.0i;
            return (int)z;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("imaginary constants are a GNU extension");
}

TEST(Compiler, isoStdAcceptsBoolTypeofAndGeneric) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            bool b;
            typeof(int) n;
            b = true;
            n = _Generic(0, int: 4, default: 0);
            printf("%d %d", (int)b, n);
            return 0;
        }
    )prg", {"-std=c"}};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, gnuStdExplicitKeepsStatementExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", ({ 8; }));
            return 0;
        }
    )prg", {"-std=gnu"}};
    program.compile();
    program.runAndExpect("8");
}

// Fuzzer: statement-expr result aliased the last lvalue, so
// ({ a = a + 1; a; }) + ({ a = a + 1; a; }) added the final `a` twice.
TEST(Compiler, gnuStatementExprResultCopiedBeforeNextOperand) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int x;
            a = 2;
            x = ({ a = a + 1; a; }) + ({ a = a + 1; a; });
            printf("%d %d", x, a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 4");
}

TEST(Compiler, gnuStatementExprResultCopiedInRelational) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int x;
            a = 1;
            x = ({ a = a + 1; a; }) < ({ a = a + 1; a; });
            printf("%d %d", x, a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, gnuStatementExprYieldsStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int x; int y; };
        int main() {
            struct P p;
            p.x = 3;
            p.y = 4;
            printf("%d", ({ p; }).x + ({ p; }).y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, gnuStatementExprLvalueAssignmentWritesThrough) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            a = 1;
            ({ a; }) = 5;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, gnuStatementExprDerefLvalueAssignmentWritesThrough) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a;
            int *p;
            a = 1;
            p = &a;
            ({ *p; }) = 5;
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, gnuStatementExprMemberLvalueAssignmentWritesThrough) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S s;
            s.x = 1;
            ({ s.x; }) = 5;
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, gnuStatementExprBitFieldAssignmentWritesThrough) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a:3; int b:5; };
        int main() {
            struct S s;
            s.a = 1;
            s.b = 5;
            ({ s.a; }) = 3;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5");
}

TEST(Compiler, gnuStatementExprBitFieldIncrementWritesThrough) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a:4; int b:4; };
        int main() {
            struct S s;
            s.a = 3;
            s.b = 7;
            printf("%d ", ++({ s.a; }));
            printf("%d ", ({ s.a; })++);
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4 5 7");
}

} // namespace
