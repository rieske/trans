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

TEST(Compiler, switchNestedInLoop) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int s;
            s = 0;
            for (i = 0; i < 4; i = i + 1) {
                switch (i) {
                    case 0:
                        s = s + 1;
                        break;
                    case 1:
                        s = s + 10;
                        break;
                    case 2:
                        continue;
                    default:
                        s = s + 100;
                }
            }
            printf("%d", s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("111");
}

TEST(Compiler, switchCaseZero) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            switch (a) {
                case 0:
                    printf("%d", 5);
                    break;
                default:
                    printf("%d", 6);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "5");
    program.runAndExpect("1", "6");
}

TEST(Compiler, switchMultipleLabels) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            switch (a) {
                case 1:
                case 2:
                    printf("%d", 12);
                    break;
                case 3:
                    printf("%d", 3);
                    break;
                default:
                    printf("%d", 0);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1", "12");
    program.runAndExpect("2", "12");
    program.runAndExpect("3", "3");
    program.runAndExpect("4", "0");
}

// Octal character escapes in case labels (git convert.c: case '\033').
TEST(Compiler, switchOctalCharEscape) {
    SourceProgram program{R"prg(
        int main() {
            int c;
            c = 27;
            switch (c) {
                case '\b':
                    printf("b");
                    break;
                case '\033':
                    printf("esc");
                    break;
                case '\014':
                    printf("ff");
                    break;
                default:
                    printf("other");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("esc");
}

// Hex character escapes in case labels (e.g. case '\x1b').
TEST(Compiler, switchHexCharEscape) {
    SourceProgram program{R"prg(
        int main() {
            int c;
            c = 27;
            switch (c) {
                case '\x1b':
                    printf("esc");
                    break;
                default:
                    printf("other");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("esc");
}

} // namespace
