#include "TestFixtures.h"

namespace {

TEST(Compiler, assignArrayToPointerReadsFirstElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = a;
            printf("%d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5 6");
}

TEST(Compiler, storeThroughDecayedPointerWritesArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            int *p;
            a[0] = 1;
            a[1] = 2;
            p = a;
            p[1] = 9;
            printf("%d %d", a[0], a[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 9");
}

TEST(Compiler, comparePointerToArrayBothOrders) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            int *p;
            a[0] = 0;
            a[1] = 0;
            p = a;
            printf("%d %d %d", p == a, a == p, p != a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 0");
}

TEST(Compiler, comparePointerToOtherArrayIsUnequal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            int b[2];
            int *p;
            a[0] = 0;
            b[0] = 0;
            p = a;
            printf("%d %d", p == b, a == b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, gitStrbufSlopCompareShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            char *buf;
        };
        int main() {
            struct strbuf sb;
            slop[0] = 0;
            sb.buf = slop;
            printf("%d %d", sb.buf != slop, slop[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, assignArrayToVoidPointerAndCastBack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            void *v;
            int *p;
            a[0] = 3;
            a[1] = 8;
            v = a;
            p = v;
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 8");
}

TEST(Compiler, returnFileScopeArrayAsPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int slop[2];
        int *addr(void) {
            return slop;
        }
        int main() {
            int *p;
            slop[0] = 1;
            slop[1] = 2;
            p = addr();
            printf("%d %d", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, passArrayToPointerParameter) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int first(int *p) {
            return p[0];
        }
        int main() {
            int a[2];
            a[0] = 11;
            a[1] = 12;
            printf("%d", first(a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, pointerArithmeticOnArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a + 1;
            printf("%d %d", *p, *(a + 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3");
}

TEST(Compiler, sizeofArrayStillDoesNotDecay) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 0;
            p = a;
            printf("%d %d", (int)sizeof(a), (int)sizeof(p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 8");
}

TEST(Compiler, addressOfArrayIsPointerToArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            a[0] = 0;
            printf("%d %d", (int)sizeof(a), (int)sizeof(&a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12 8");
}

TEST(Compiler, assignToArrayIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a[2];
            int b[2];
            a[0] = 0;
            b[0] = 0;
            a = b;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

} // namespace
