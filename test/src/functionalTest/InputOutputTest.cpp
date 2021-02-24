#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesSimpleOutputProgram) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            printf("%d ", a);
            printf("%d ", -a);
            printf("%d ", -(-a));
            printf("%d", 2-5);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1 -1 1 -3");
}

TEST(Compiler, inputOutput) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            input a;
            printf("%d", a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42", "42");
    program.runAndExpect("-42", "-42");
}

TEST(Compiler, printfConstant) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", 42);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42");
}

TEST(Compiler, printfHelloWorld) {
    SourceProgram program{R"prg(
        int main() {
            printf("Hello, World!");
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("Hello, World!");
}

TEST(Compiler, printfHelloWorldWithNewLine) {
    SourceProgram program{R"prg(
        int main() {
            printf("Hello, World!\n");
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("Hello, World!\n");
}

TEST(Compiler, printfHelloWorldWithNewLineMidString) {
    SourceProgram program{R"prg(
        int main() {
            printf("Hello\nWorld!");
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("Hello\nWorld!");
}

TEST(Compiler, printfInteger) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            printf("%d", a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1");
}

TEST(Compiler, printfMultipleIntegers) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            int b = 42;
            printf("int1: %d\nint2: %d\n", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("int1: 1\nint2: 42\n");
}

TEST(Compiler, printfNegativeIntegers) {
    SourceProgram program{R"prg(
        int main() {
            int a = -1;
            int b = -42;
            printf("int1: %d\nint2: %d\n", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("int1: -1\nint2: -42\n");
}

}

