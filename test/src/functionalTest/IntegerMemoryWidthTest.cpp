#include "TestFixtures.h"

namespace {

// Named local, dirty 8 bytes at &obj, store through a typed pointer, then if / == 0.

TEST(Compiler, boolStoredThroughPointerThenIfUsesOnlyThatByte) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            bool b;
            dirty8(&b);
            set(&b, 0);
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            set(&b, 1);
            if (b) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, boolStoredThroughPointerThenEqualsZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void set(bool *p, int v) {
            *p = v;
        }
        int main() {
            bool b;
            dirty8(&b);
            set(&b, 0);
            printf("%d ", b == 0);
            set(&b, 1);
            printf("%d", b != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, charStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setc(char *p, int v) {
            *p = v;
        }
        int main() {
            char c;
            dirty8(&c);
            setc(&c, 0);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d ", c == 0);
            setc(&c, -1);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 1");
}

TEST(Compiler, unsignedCharStoredThroughPointerThenIf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void setuc(unsigned char *p, int v) {
            *p = v;
        }
        int main() {
            unsigned char c;
            dirty8(&c);
            setuc(&c, 0);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            setuc(&c, 255);
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("01");
}

TEST(Compiler, shortStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void sets(short *p, int v) {
            *p = v;
        }
        int main() {
            short s;
            dirty8(&s);
            sets(&s, 0);
            if (s) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", s == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, manyLiveCharsThenDereferenceDoesNotCrashCompiler) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int sum(char *p) {
            char a0 = p[0];
            char a1 = p[1];
            char a2 = p[2];
            char a3 = p[3];
            char a4 = p[4];
            char a5 = p[5];
            char a6 = p[6];
            char a7 = p[7];
            char a8 = p[8];
            char a9 = p[9];
            char a10 = p[10];
            char a11 = p[11];
            char a12 = p[12];
            char a13 = p[13];
            char a14 = p[14];
            char a15 = p[15];
            char x = *p;
            return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
                    + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + x;
        }
        int main() {
            char buf[16];
            int i;
            for (i = 0; i < 16; i++) {
                buf[i] = (char)(i + 1);
            }
            printf("%d", sum(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("137");
}

TEST(Compiler, manyLiveShortsThenDereferenceDoesNotCrashCompiler) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int sum(short *p) {
            short a0 = p[0];
            short a1 = p[1];
            short a2 = p[2];
            short a3 = p[3];
            short a4 = p[4];
            short a5 = p[5];
            short a6 = p[6];
            short a7 = p[7];
            short a8 = p[8];
            short a9 = p[9];
            short a10 = p[10];
            short a11 = p[11];
            short a12 = p[12];
            short a13 = p[13];
            short a14 = p[14];
            short a15 = p[15];
            short x = *p;
            return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7
                    + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15 + x;
        }
        int main() {
            short buf[16];
            int i;
            for (i = 0; i < 16; i++) {
                buf[i] = (short)(i + 1);
            }
            printf("%d", sum(buf));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("137");
}

TEST(Compiler, intStoredThroughPointerThenIfIgnoresNeighborBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void dirty8(void *p) {
            unsigned long *w;
            w = (unsigned long *)p;
            *w = 0xffffffffffffffffUL;
        }
        void seti(int *p, int v) {
            *p = v;
        }
        int main() {
            int n;
            dirty8(&n);
            seti(&n, 0);
            if (n) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", n == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, unsignedCharIncrementWrapsThenIfIsFalse) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 255;
            c++;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", c == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, unsignedCharDecrementWrapsThenIfIsTrue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            unsigned char c;
            c = 0;
            c--;
            if (c) {
                printf("1");
            } else {
                printf("0");
            }
            printf(" %d", (int)c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 255");
}

TEST(Compiler, intIncrementSignExtendsWhenUsedAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            n = -2;
            n++;
            printf("%ld", (long)n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

} // namespace
