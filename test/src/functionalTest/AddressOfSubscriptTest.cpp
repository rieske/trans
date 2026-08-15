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

TEST(Compiler, addressOfDerefIsIdentityWithoutLoad) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int *p = (int *)0x100;
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
