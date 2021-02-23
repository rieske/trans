#include "TestFixtures.h"

namespace {

TEST(Compiler, compilesSimpleOutputProgram) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            output a;
            output -a;
            output -(-a);
            output 2-5;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1\n-1\n1\n-3\n");
}

TEST(Compiler, inputOutput) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            input a;
            output a;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42", "42\n");
    program.runAndExpect("-42", "-42\n");
}

TEST(Compiler, outputConstant) {
    SourceProgram program{R"prg(
        int main() {
            output 42;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42\n");
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
            printf("int1 %d\nint2 %d\n", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("int1 1\nint2 42\n");
}

TEST(Compiler, printfNegativeIntegers) {
    SourceProgram program{R"prg(
        int main() {
            int a = -1;
            int b = -42;
            printf("int1 %d\nint2 %d\n", a, b);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("int1 -1\nint2 -42\n");
}

/*TEST(Compiler, printfMultipleIntegersWithColons) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            int b = 42;
            printf("int1: %d\nint2: %d\n", a, b);
            return 0;
        }
    )prg", "printf"};

    // FIXME: looks like a bug in scanner: Error: Can't reach next state for given input: :
    program.compile(true);

    program.runAndExpect("int1: 1\nint2: 42\n");
}*/

}

