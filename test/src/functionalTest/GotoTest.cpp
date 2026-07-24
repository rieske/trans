#include "TestFixtures.h"

namespace {

TEST(Compiler, gotoForward) {
    SourceProgram program{R"prg(
        int main() {
            goto skip;
            printf("%d", 1);
        skip:
            printf("%d", 2);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, gotoBackward) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
        loop:
            n = n + 1;
            if (n < 3) {
                goto loop;
            }
            printf("%d", n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, gotoSkipsOver) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 0;
            goto end;
            a = 1;
        end:
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, gotoUndefinedLabelIsError) {
    SourceProgram program{R"prg(
        int main() {
            goto missing;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("label `missing` used but not defined");
}

TEST(Compiler, gotoDuplicateLabelIsError) {
    SourceProgram program{R"prg(
        int main() {
        lab:
            printf("%d", 1);
        lab:
            printf("%d", 2);
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("duplicate label");
}

} // namespace
