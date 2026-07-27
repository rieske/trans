#include "TestFixtures.h"

namespace {

TEST(Compiler, forwardDeclarationThenDefinition) {
    SourceProgram program{R"prg(
        int foo(int x);

        int main() {
            printf("%d", foo(3));
            return 0;
        }

        int foo(int x) {
            return x + 1;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, mutualRecursion) {
    SourceProgram program{R"prg(
        int isOdd(int n);

        int isEven(int n) {
            if (n) {
                return isOdd(n - 1);
            } else {
                return 1;
            }
        }

        int isOdd(int n) {
            if (n) {
                return isEven(n - 1);
            } else {
                return 0;
            }
        }

        int main() {
            printf("%d %d %d %d", isEven(0), isEven(1), isOdd(0), isOdd(3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 0 1");
}

TEST(Compiler, prototypeOnlyThenCallAfterDefinition) {
    SourceProgram program{R"prg(
        int bar(int a, int b);

        int bar(int a, int b) {
            return a * b;
        }

        int main() {
            printf("%d", bar(6, 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unsignedIntPrototypeMatchesUnsigned) {
    SourceProgram program{R"prg(
        unsigned f(void);
        unsigned int f(void) {
            return 7;
        }
        unsigned int g(unsigned int x);
        unsigned g(unsigned x) {
            return x + 1;
        }
        int main() {
            printf("%d %d", f(), g(41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 42");
}

TEST(Compiler, incompleteStructForwardThenComplete) {
    SourceProgram program{R"prg(
        struct Node;
        struct Node {
            int v;
            struct Node *next;
        };
        int main() {
            struct Node n;
            n.v = 3;
            n.next = 0;
            printf("%d", n.v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


TEST(Compiler, incompatiblePrototypeDefinitionIsError) {
    SourceProgram program{R"prg(
        int f(int x);
        int f(void) {
            return 1;
        }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with previous");
}

TEST(Compiler, duplicateFunctionDefinitionIsError) {
    SourceProgram program{R"prg(
        int f(void) {
            return 1;
        }
        int f(void) {
            return 2;
        }
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("conflicts with previous");
}

} // namespace
