#include "TestFixtures.h"

// &E[i] and &*E are pointer arithmetic. They must not load the object.

namespace {

TEST(Compiler, addressOfHugePointerIndexDoesNotLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned int buf[2];
            unsigned int *p = buf;
            unsigned int i = 0x7F000000u;
            unsigned int *q = &p[i];
            if (q != (unsigned int *)((char *)p + (unsigned long)i * 4ul))
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, addressOfHugeIndexCheckDoesNotLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int check(const void *start, unsigned long size, const void *ptr) {
            const unsigned char *s = start;
            const unsigned char *e = s + size;
            const unsigned char *q = ptr;
            if (q < s)
                return 1;
            if (q >= e - 8)
                return 2;
            return 0;
        }
        int main(void) {
            unsigned int idx[4];
            unsigned int *idx2 = idx;
            unsigned int off = 0x7F000000u;
            if (check(idx, sizeof idx, &idx2[off * 2]) != 2)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// A load of *p would fault. Completing with q == p means &*p did not load.
TEST(Compiler, addressOfDerefNullIsIdentityWithoutLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int *p = 0;
            int *q = &*p;
            if (q != p)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// &*E is E. The pointer must be the operand, not an uninitialized deref temp
// (git SWAP(*a,*b) / strbuf_swap).
TEST(Compiler, addressOfDerefStructIsIdentity) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { unsigned long a; unsigned long b; char *p; };
        int main(void) {
            struct S s;
            struct S *ps = &s;
            struct S *q = &*ps;
            if (q != ps)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, addressOfDerefStackIntIsIdentity) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            int *p = &x;
            int *q = &*p;
            if (q != p)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// &*arr is &arr[0]. Must not treat the array object home as a pointer value.
TEST(Compiler, addressOfDerefArrayIsIdentity) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char color[16];
            char *q = &*color;
            if (q != color)
                return 1;
            *color = 88;
            if (*color != 88)
                return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// *p++ stores at the old pointer; p advances. Result of p++ is not p itself.
TEST(Compiler, derefPostfixAssignUsesOldPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int a[2];
            int *p;
            a[0] = 1;
            a[1] = 2;
            p = a;
            *p++ = 7;
            if (a[0] != 7 || a[1] != 2 || p != a + 1)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, pointerSubscriptStillLoadsInRange) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned int buf[2];
            unsigned int *p = buf;
            buf[0] = 3;
            buf[1] = 5;
            p[0] = 7;
            if (p[0] != 7 || p[1] != 5)
                return 1;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

} // namespace
