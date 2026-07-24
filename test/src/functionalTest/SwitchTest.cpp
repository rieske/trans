#include "TestFixtures.h"

namespace {

TEST(Compiler, switchBasicCases) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            switch (a) {
                case 1:
                    printf("%d", 10);
                    break;
                case 2:
                    printf("%d", 20);
                    break;
                case 3:
                    printf("%d", 30);
                    break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1", "10");
    program.runAndExpect("2", "20");
    program.runAndExpect("3", "30");
}

TEST(Compiler, switchDefault) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            switch (a) {
                case 1:
                    printf("%d", 1);
                    break;
                default:
                    printf("%d", 0);
                    break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1", "1");
    program.runAndExpect("0", "0");
    program.runAndExpect("99", "0");
}

TEST(Compiler, switchFallThrough) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int s;
            s = 0;
            scanf("%ld", &a);
            switch (a) {
                case 1:
                    s = s + 1;
                case 2:
                    s = s + 2;
                    break;
                case 3:
                    s = s + 4;
                    break;
            }
            printf("%d", s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1", "3");
    program.runAndExpect("2", "2");
    program.runAndExpect("3", "4");
}

TEST(Compiler, switchNoMatchNoDefault) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            switch (a) {
                case 1:
                    printf("%d", 1);
                    break;
            }
            printf("%d", 9);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2", "9");
    program.runAndExpect("1", "19");
}

TEST(Compiler, switchBreakExits) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            switch (a) {
                case 1:
                    printf("%d", 1);
                    break;
                    printf("%d", 2);
                default:
                    printf("%d", 3);
            }
            printf("%d", 4);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("14");
}

TEST(Compiler, switchCaseOutsideIsError) {
    SourceProgram program{R"prg(
        int main() {
            case 1:
                return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("case label not within a switch");
}

TEST(Compiler, switchMultipleDefaultIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            switch (a) {
                default:
                    printf("%d", 1);
                    break;
                default:
                    printf("%d", 2);
                    break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("multiple default labels");
}

// continue is only valid in a loop, not in a bare switch (cont is null on switch frame).
TEST(Compiler, continueInsideSwitchIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            switch (a) {
                case 1:
                    continue;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("`continue` statement not in loop");
}

TEST(Compiler, switchDuplicateCaseIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            a = 1;
            switch (a) {
                case 1:
                    printf("%d", 1);
                    break;
                case 1:
                    printf("%d", 2);
                    break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("duplicate case value");
}

// continue in a loop that contains a switch still targets the loop.
TEST(Compiler, continueInLoopAroundSwitch) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int sum;
            i = 0;
            sum = 0;
            while (i < 3) {
                i = i + 1;
                switch (i) {
                    case 2:
                        continue;
                    default:
                        sum = sum + i;
                        break;
                }
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    // i=1 sum=1; i=2 continue; i=3 sum=4
    program.runAndExpect("4");
}

} // namespace
