#include "TestFixtures.h"

#include <string>

// Implicit { 0 } / omitted designated members zero the whole object
// representation, including struct padding.

namespace {

const char kPad[] = R"c(
        struct Pad {
            void *p0;
            void *p1;
            void *p2;
            void *p3;
            void *p4;
            void *p5;
            char c;
            int i;
        };
        static struct Pad zeros;
)c";

std::string dirtyStackProgram(const std::string& decls, const std::string& checkBody) {
    return std::string(R"c(int printf(const char *, ...);
int memcmp(const void *, const void *, unsigned long);
)c") + decls + R"c(
        int dirty(void) {
            volatile unsigned char junk[8192];
            int i;
            for (i = 0; i < 8192; ++i)
                junk[i] = 0xaa;
            return junk[8191];
        }
        int check(void) {
)c" + checkBody + R"c(
        }
        int main(void) {
            int n;
            for (n = 0; n < 8; ++n) {
                dirty();
                if (check())
                    return 1;
            }
            printf("ok");
            return 0;
        }
)c";
}

void compileAndRunOk(const std::string& source) {
    SourceProgram program{source};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, localZeroInitZerosCharIntPadding) {
    compileAndRunOk(dirtyStackProgram(kPad, R"c(
            struct Pad x = { 0 };
            if ((int)((char *)&zeros.i - (char *)&zeros.c) <= 1)
                return 2;
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, localZeroInitZerosEnumBeforeUnionPadding) {
    compileAndRunOk(dirtyStackProgram(R"c(
        struct Info {
            void *a;
            void *b;
            void *c;
            void *d;
            void *e;
            void *f;
            enum { W0, W1, W2 } whence;
            union {
                struct {
                    void *pack;
                    long offset;
                    enum { T0, T1 } type;
                } packed;
            } u;
        };
        static struct Info zeros;
)c",
            R"c(
            struct Info x = { 0 };
            if ((int)((char *)&zeros.u - (char *)&zeros.whence) <= (int)sizeof zeros.whence)
                return 2;
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, designatedOmitZerosPadding) {
    compileAndRunOk(dirtyStackProgram(kPad, R"c(
            struct Pad x = { .i = 0 };
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, nestedStructZeroInitZerosInnerPadding) {
    compileAndRunOk(dirtyStackProgram(R"c(
        struct Inner {
            void *p0;
            void *p1;
            void *p2;
            void *p3;
            void *p4;
            void *p5;
            char c;
            int i;
        };
        struct Outer {
            struct Inner in;
            int x;
        };
        static struct Outer zeros;
)c",
            R"c(
            struct Outer x = { 0 };
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, compoundLiteralZeroInitZerosPadding) {
    compileAndRunOk(dirtyStackProgram(kPad, R"c(
            return memcmp(&(struct Pad){ 0 }, &zeros, sizeof(struct Pad)) != 0;
)c"));
}

TEST(Compiler, nestedDesignatorLeavesSiblingPaddingZero) {
    compileAndRunOk(dirtyStackProgram(R"c(
        struct Inner {
            void *p0;
            void *p1;
            void *p2;
            void *p3;
            void *p4;
            void *p5;
            char c;
            int i;
        };
        struct Outer {
            struct Inner in;
            int x;
        };
        static struct Outer zeros;
)c",
            R"c(
            struct Outer x = { .in.i = 7 };
            if (x.in.i != 7)
                return 3;
            x.in.i = 0;
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, unionZeroInitZerosRepresentation) {
    compileAndRunOk(dirtyStackProgram(R"c(
        union U {
            char c;
            long l;
        };
        static union U zeros;
)c",
            R"c(
            union U x = { 0 };
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, arrayPartialDesignatorZerosOmittedPadding) {
    compileAndRunOk(dirtyStackProgram(R"c(
        struct Pad {
            void *p0;
            void *p1;
            void *p2;
            void *p3;
            void *p4;
            void *p5;
            char c;
            int i;
        };
        static struct Pad zeros[2];
)c",
            R"c(
            struct Pad x[2] = { [1] = { .i = 3 } };
            if (x[1].i != 3)
                return 3;
            x[1].i = 0;
            return memcmp(x, zeros, sizeof x) != 0;
)c"));
}

TEST(Compiler, designatorThenPositionalZerosPadding) {
    compileAndRunOk(dirtyStackProgram(kPad, R"c(
            struct Pad x = { .c = 1, 2 };
            if ((int)x.c != 1 || x.i != 2)
                return 3;
            x.c = 0;
            x.i = 0;
            return memcmp(&x, &zeros, sizeof x) != 0;
)c"));
}

} // namespace
