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

/*TEST(Compiler, printfHelloWorld) {
    SourceProgram program{R"prg(
        int main() {
            printf("Hello, World!");
            return 0;
        }
    )prg", "printf"};

    program.compile(true);

    program.runAndExpect("Hello, World!");
}*/

/*TEST(Compiler, printfInteger) {
    SourceProgram program{R"prg(
        int main() {
            int a = 1;
            printf("%d", a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1");
}*/

}

