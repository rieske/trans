#include "TestFixtures.h"

namespace {

TEST(Compiler, simpleFunctionPointer) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

} // namespace
