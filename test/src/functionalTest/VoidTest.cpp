#include "TestFixtures.h"

namespace {

TEST(Compiler, voidReturnExplicit) {
    SourceProgram program{R"prg(
        void voidRet() {
            return;
        }

        int main() {
            voidRet();
            return 0;
        }
    )prg"};

    program.compile();

    program.run();
}

TEST(Compiler, voidReturnImplicit) {
    SourceProgram program{R"prg(
        void voidRet() {
        }

        int main() {
            voidRet();
            return 0;
        }
    )prg"};

    program.compile();

    program.run();
}

TEST(Compiler, voidReturnEmptyConditionalBranch) {
    SourceProgram program{R"prg(
        void voidRet(int shouldReturn) {
            if (shouldReturn == 1) {
                return;
            } else {
            }
        }

        int main() {
            int shouldReturn;
            scanf("%d", &shouldReturn);
            voidRet(shouldReturn);
            return 0;
        }
    )prg"};

    program.compile();

    program.run("1");
    program.run("0");
}

TEST(Compiler, voidReturnConditional) {
    SourceProgram program{R"prg(
        void voidRet(int shouldReturn) {
            if (shouldReturn == 1) {
                return;
            }
        }

        int main() {
            int shouldReturn;
            scanf("%d", &shouldReturn);
            voidRet(shouldReturn);
            return 0;
        }
    )prg"};

    program.compile();

    program.run("1");
    program.run("0");
}

TEST(Compiler, voidOutput) {
    SourceProgram program{R"prg(
        void voidOutput(int value) {
            printf("%d", value);
        }

        int main() {
            int value;
            scanf("%d", &value);
            voidOutput(value);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "1");
    program.runAndExpect("42", "42");
}

}

