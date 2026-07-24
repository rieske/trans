#include "TestFixtures.h"

namespace {

TEST(Compiler, localArrayReadWrite) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, arrayIndexExpression) {
    SourceProgram program{R"prg(
        int main() {
            int a[5];
            int i;
            for (i = 0; i < 5; i = i + 1) {
                a[i] = i * 10;
            }
            printf("%d %d %d", a[0], a[2], a[4]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 20 40");
}

TEST(Compiler, arrayOfPointers) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            int y;
            int* p[2];
            x = 11;
            y = 22;
            p[0] = &x;
            p[1] = &y;
            printf("%d %d", *p[0], *p[1]);
            *p[1] = 33;
            printf(" %d", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 22 33");
}

TEST(Compiler, pointerSubscript) {
    SourceProgram program{R"prg(
        int main() {
            int a[3];
            int* p;
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = &a[0];
            printf("%d %d", p[1], p[2]);
            p[0] = 9;
            printf(" %d", a[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6 9");
}

TEST(Compiler, charArray) {
    SourceProgram program{R"prg(
        int main() {
            char s[3];
            s[0] = 65;
            s[1] = 66;
            s[2] = 67;
            printf("%d %d %d", s[0], s[1], s[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65 66 67");
}

TEST(Compiler, addressOfArrayElement) {
    SourceProgram program{R"prg(
        int main() {
            int a[2];
            int* p;
            a[0] = 7;
            a[1] = 8;
            p = &a[1];
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

} // namespace
