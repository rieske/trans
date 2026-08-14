#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int one() {
            return 1;
        }

        int main() {
            int (*p)();
            p = one;
            printf("%d", p());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, addressOfFunctionDesignator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int two() {
            return 2;
        }

        int main() {
            int (*p)();
            p = &two;
            printf("%d", p());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, functionPointerReassign) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int a() { return 3; }
        int b() { return 4; }

        int main() {
            int (*p)();
            p = a;
            printf("%d ", p());
            p = b;
            printf("%d", p());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, functionPointerWithArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int add1(int x) {
            return x + 1;
        }

        int main() {
            int (*fp)(int);
            fp = add1;
            printf("%d", fp(5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, callThroughStarFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int three() {
            return 3;
        }

        int main() {
            int (*p)();
            p = three;
            printf("%d", (*p)());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, functionPointerInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int five() { return 5; }
        int main() {
            int (*fp)() = five;
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, designatorInitToIntIsError) {
    SourceProgram program{R"prg(
        int foo() { return 1; }
        int main() {
            int a = foo;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, designatorReturnIsError) {
    SourceProgram program{R"prg(
        int foo() { return 1; }
        int bad() { return foo; }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, indirectCallWithSevenArgs) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int sum7(int a, int b, int c, int d, int e, int f, int g) {
            return a + b + c + d + e + f + g;
        }
        int main() {
            int (*fp)(int, int, int, int, int, int, int);
            fp = sum7;
            printf("%d", fp(1, 2, 3, 4, 5, 6, 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("28");
}

TEST(Compiler, doublePointerNotCallable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int one() { return 1; }
        int main() {
            int (*fp)();
            int (**pp)();
            fp = one;
            pp = &fp;
            printf("%d", (*pp)());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, addressOfDesignatorToIntIsError) {
    SourceProgram program{R"prg(
        int foo() { return 1; }
        int main() {
            int a;
            a = &foo;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

// Non-identifier callee → unscopedSymbolName + "not a function" (Coveralls path).
TEST(Compiler, callThroughNonFunctionPointerIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int *p;
            p = &x;
            (*p)();
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("is not a function");
}

TEST(Compiler, callArrayElementIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a[1];
            a[0]();
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("is not a function");
}

TEST(Compiler, callIntIdentifierIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a();
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("is not a function");
}

// Assignment expression result is a scoped local ($s…); strips via unscopedSymbolName.
TEST(Compiler, callAssignmentExpressionIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            (a = 1)();
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("is not a function");
}

// Direct call of a designator (not through a pointer variable).
TEST(Compiler, directDesignatorCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int six() { return 6; }
        int main() {
            printf("%d", six());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, functionPointerPassedAsArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int seven() { return 7; }
        int apply(int (*fp)()) {
            return fp();
        }
        int main() {
            printf("%d", apply(seven));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, functionPointerStoredInLocalAndCalled) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int eight() { return 8; }
        int nine() { return 9; }
        int main() {
            int (*table)();
            table = eight;
            printf("%d ", table());
            table = nine;
            printf("%d", table());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 9");
}

// Null pointer constant includes an integer constant cast to void* (typical NULL).
// Prefer !fp over fp == 0 (fnptr vs integer comparison is a separate product gap).
TEST(Compiler, functionPointerAssignedFromVoidStarZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int one() { return 1; }
        int main() {
            int (*fp)();
            fp = one;
            fp = (void *)0;
            printf("%d", !fp);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, functionPointerAssignedFromNullMacro) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define NULL ((void *)0)
        int one() { return 1; }
        int main() {
            int (*fp)();
            fp = one;
            fp = NULL;
            printf("%d", !fp);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, functionPointerArgNullConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define NULL ((void *)0)
        int call_or_zero(int (*fp)()) {
            if (!fp) {
                return 0;
            }
            return fp();
        }
        int five() { return 5; }
        int main() {
            printf("%d ", call_or_zero(NULL));
            printf("%d", call_or_zero(five));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 5");
}

TEST(Compiler, functionPointerStructFieldNullInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define NULL ((void *)0)
        struct S {
            int (*callback)();
        };
        int seven() { return 7; }
        int main() {
            struct S s;
            s.callback = seven;
            s.callback = NULL;
            printf("%d ", !s.callback);
            s.callback = seven;
            printf("%d", s.callback());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7");
}

TEST(Compiler, callThroughStarStructFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int (*callback)();
        };
        int seven() { return 7; }
        int main() {
            struct S s;
            s.callback = seven;
            printf("%d", (*s.callback)());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, callThroughStarArrayFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int seven() { return 7; }
        int main() {
            int (*a[1])();
            a[0] = seven;
            printf("%d", (*a[0])());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, callThroughStarAddressOfFunction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int seven() { return 7; }
        int main() {
            printf("%d", (*&seven)());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, functionPointerStructDesignatedNullInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define NULL ((void *)0)
        struct S {
            int (*callback)();
            int tag;
        };
        int main() {
            struct S s = { .callback = NULL, .tag = 3 };
            printf("%d %d", !s.callback, s.tag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

// Non-null void* is not a null pointer constant and must not convert to function pointer.
TEST(Compiler, functionPointerFromNonNullVoidStarIsError) {
    SourceProgram program{R"prg(
        int main() {
            void *p;
            int (*fp)();
            p = 0;
            fp = p;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// C 6.7.6.3: parameter declared as function type is adjusted to pointer-to-function.
TEST(Compiler, functionTypeParameterAdjustedToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef int each_fn(int x);
        int apply(each_fn fn, int x) {
            return fn(x);
        }
        int add_one(int x) {
            return x + 1;
        }
        int main() {
            printf("%d", apply(add_one, 41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, bareFunctionTypeParameterAdjusted) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int apply(int fn(int), int x) {
            return fn(x);
        }
        int times_two(int x) {
            return x * 2;
        }
        int main() {
            printf("%d", apply(times_two, 21));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// Adjusted parameters are function pointers: pass and call without treating them as designators.
TEST(Compiler, functionTypeParameterPassedAndCalled) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef int cb_t(int);
        int invoke(cb_t fn) {
            return fn(6);
        }
        int pass_through(cb_t fn) {
            return invoke(fn);
        }
        int identity(int x) {
            return x;
        }
        int main() {
            printf("%d", pass_through(identity));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, functionTypeParameterNullCheck) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define NULL ((void *)0)
        typedef int cb_t(void);
        int is_null(cb_t fn) {
            return !fn;
        }
        int one(void) {
            return 1;
        }
        int main() {
            printf("%d ", is_null(NULL));
            printf("%d", is_null(one));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

// `int (*f(int which))(int)`: the defined function's parameter is `which`,
// not the unnamed parameter of the returned function type.
TEST(Compiler, functionReturningFunctionPointerBindsParameterName) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int add1(int x) { return x + 1; }
        int add2(int x) { return x + 2; }
        int (*pick_fp(int which))(int) { return which ? add2 : add1; }
        int main() {
            int (*fp)(int);
            fp = pick_fp(0);
            printf("%d", fp(2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, functionReturningFunctionPointerSelectsByParameter) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int add1(int x) { return x + 1; }
        int add2(int x) { return x + 2; }
        int (*pick_fp(int which))(int) { return which ? add2 : add1; }
        int main() {
            printf("%d %d", pick_fp(0)(2), pick_fp(1)(2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, functionReturningPointerToArrayBindsParameterName) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int g[2][2] = { { 1, 2 }, { 3, 4 } };
        int (*row(int which))[2] { return g + which; }
        int main() {
            int (*p)[2];
            p = row(1);
            printf("%d %d", (*p)[0], (*p)[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

} // namespace
