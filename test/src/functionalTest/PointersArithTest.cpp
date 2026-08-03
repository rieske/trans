#include "TestFixtures.h"

namespace {

// Pointer + large scale element (struct array stride > 1).
TEST(Compiler, pointerArithOnStructArray) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Cell { int a; int b; };
        int main() {
            struct Cell cells[3];
            cells[0].a = 1; cells[0].b = 2;
            cells[1].a = 10; cells[1].b = 20;
            cells[2].a = 100; cells[2].b = 200;
            struct Cell *p = cells;
            p = p + 2;
            printf("%d", p->a + p->b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("300");
}

// Pointer difference (ptrdiff plan).
TEST(Compiler, pointerDifference) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[10];
            int *p = &a[7];
            int *q = &a[3];
            printf("%ld", (long)(p - q));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

// git portable obstack_free(h, NULL): tempint = (char*)0 - (char*)chunk, then
// chunk + tempint must be NULL. 32-bit ptrdiff truncates high heap addresses.
TEST(Compiler, charPointerDifferenceIs64BitPtrdiff) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char *chunk;
            char *obj;
            long tempint;
            chunk = (char *)0x55555890c890UL;
            obj = 0;
            tempint = obj - chunk;
            printf("%d", (tempint + chunk) == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Pointer arithmetic must scale by pointee size (int stride 4). Found by targeted probing
// during mutfuzz campaign: p+1 / p-q used raw byte math.

TEST(Compiler, pointerPlusIntScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(p + 1));
            printf("%d", *(p + 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

TEST(Compiler, pointerMinusPointerIsElementCount) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            int *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerIncrementScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            p++;
            printf("%d ", *p);
            ++p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// int + ptr is commutative with ptr + int (IntPlusPtr IR form).
TEST(Compiler, intPlusPointerScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[0];
            printf("%d ", *(1 + p));
            printf("%d", *(2 + p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 30");
}

// ptr - int scales the index (PtrMinusInt IR form).
TEST(Compiler, pointerMinusIntScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            printf("%d ", *(p - 1));
            printf("%d", *(p - 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

// Pointer -- / prefix -- step by pointee size (non-unit Dec quad).
TEST(Compiler, pointerDecrementScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = &a[2];
            p--;
            printf("%d ", *p);
            --p;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 10");
}

TEST(Compiler, charPointerDifferenceIsBytes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            char a[3];
            char *p;
            char *q;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = &a[2];
            q = &a[0];
            printf("%d", p - q);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, pointerNegativeSubscriptIsPreviousElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char s[4];
            char *p;
            s[0] = 'a';
            s[1] = 'b';
            s[2] = 'c';
            s[3] = 0;
            p = s + 2;
            printf("%c", p[-1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("b");
}

TEST(Compiler, intPointerNegativeSubscriptScales) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a + 2;
            printf("%d", p[-1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, pointerPlusNegativeInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            int n;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            p = a + 2;
            n = -1;
            printf("%d", *(p + n));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, walkBackToGreaterThanLikeParseCommitDate) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char line[32];
            char *buf;
            char *eol;
            char *dateptr;
            line[0] = 't';
            line[1] = ' ';
            line[2] = '<';
            line[3] = 'e';
            line[4] = '>';
            line[5] = ' ';
            line[6] = '1';
            line[7] = '\n';
            line[8] = 0;
            buf = line;
            eol = line;
            while (*eol != '\n') {
                eol = eol + 1;
            }
            dateptr = eol;
            while (dateptr > buf && dateptr[-1] != '>') {
                dateptr = dateptr - 1;
            }
            printf("%c%c", dateptr[-1], dateptr[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("> ");
}

// p += n / p -= n must scale by pointee size. Fuzz: += used unscaled integer add
// (++p was already correct).
TEST(Compiler, pointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            p = &a[0];
            p += 1;
            printf("%d %d ", *p, (int)(p - a));
            p += 2;
            printf("%d %d", *p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20 1 40 3");
}

TEST(Compiler, pointerMinusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[4];
            int *p;
            a[0] = 10;
            a[1] = 20;
            a[2] = 30;
            a[3] = 40;
            p = &a[3];
            p -= 1;
            printf("%d %d ", *p, (int)(p - a));
            p -= 2;
            printf("%d %d", *p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("30 2 10 0");
}

TEST(Compiler, shortPointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            short a[3];
            short *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            p += 1;
            printf("%d %d", (int)*p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

TEST(Compiler, structPointerPlusEqualScalesByElementSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; int y; };
        int main() {
            struct S a[2];
            struct S *p;
            a[0].x = 1; a[0].y = 2;
            a[1].x = 3; a[1].y = 4;
            p = a;
            p += 1;
            printf("%d %d %d", p->x, p->y, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4 1");
}

TEST(Compiler, charPointerPlusEqualStepsByOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char a[3];
            char *p;
            a[0] = 1;
            a[1] = 2;
            a[2] = 3;
            p = a;
            p += 1;
            printf("%d %d", (int)*p, (int)(p - a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}

// C 6.5.2.1: E1[E2] is *((E1)+(E2)), so 1[p] == p[1].
TEST(Compiler, integerSubscriptOfPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            int *p;
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            p = a;
            printf("%d %d %d", 0[p], 1[p], 2[p]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 5 6");
}

TEST(Compiler, integerSubscriptOfArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            printf("%d %d", 1[a], 2[a]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}

TEST(Compiler, integerSubscriptStaticArrayAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int a[3];
        int *p = &1[a];
        int main() {
            a[0] = 4;
            a[1] = 5;
            a[2] = 6;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

} // namespace
