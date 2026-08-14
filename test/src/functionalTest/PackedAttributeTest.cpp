#include "TestFixtures.h"

// GCC/SysV: __attribute__((packed)) suppresses padding (alignment 1).

namespace {

TEST(Compiler, packedCharIntSizeofIsFive) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) S { char c; int i; };
        int main(void) {
            printf("%d", (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, packedCharIntOffsetofIntIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) S { char c; int i; };
        int main(void) {
            printf("%d", (int)__builtin_offsetof(struct S, i));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, packedShortIntSizeofIsSix) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) S { short s; int i; };
        int main(void) {
            printf("%d", (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, packedUnionSizeofIsFour) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union __attribute__((packed)) U { char c; int i; };
        int main(void) {
            printf("%d", (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, packedOuterDoesNotPackNestedInner) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) Outer {
            struct Inner { char c; int i; };
            char z;
        };
        int main(void) {
            printf("%d %d", (int)sizeof(struct Inner), (int)sizeof(struct Outer));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1");
}

TEST(Compiler, packedInnerDoesNotPackEnclosingOuter) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Outer {
            struct __attribute__((packed)) Inner { char c; int i; };
            char z;
        };
        int main(void) {
            printf("%d %d", (int)sizeof(struct Inner), (int)sizeof(struct Outer));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 1");
}

TEST(Compiler, packedAttributeAfterBracePacksType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { char c; int i; } __attribute__((packed));
        int main(void) {
            printf("%d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, i));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 1");
}

TEST(Compiler, packedTypedefAfterAnonymousStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef struct { char c; int i; } __attribute__((packed)) S;
        int main(void) {
            printf("%d %d", (int)sizeof(S), (int)__builtin_offsetof(S, i));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 1");
}

TEST(Compiler, packedOnForwardDeclDoesNotPackLaterDefinition) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) S;
        struct S { char c; int i; };
        int main(void) {
            printf("%d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, i));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 4");
}

TEST(Compiler, packedAttributeBeforeStructKeywordDoesNotPack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        __attribute__((packed)) struct S { char c; int i; };
        int main(void) {
            printf("%d %d", (int)sizeof(struct S), (int)__builtin_offsetof(struct S, i));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 4");
}

TEST(Compiler, packedStructArrayElementsDoNotOverlap) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct __attribute__((packed)) S { char c; int i; };
        int main(void) {
            struct S a[2];
            struct S s;
            s.c = 1;
            s.i = 40;
            a[1].c = 2;
            a[1].i = 50;
            a[0] = s;
            printf("%d %d %d %d", a[0].c, a[0].i, a[1].c, a[1].i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 40 2 50");
}

} // namespace
