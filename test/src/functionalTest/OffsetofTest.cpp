#include "TestFixtures.h"

namespace {

TEST(Compiler, offsetofSecondIntMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a; int b; };
        int main(void) {
            printf("%d", (int)__builtin_offsetof(struct S, b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, offsetofCharThenIntPads) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { char a; int b; };
        int main(void) {
            printf("%d %d", (int)__builtin_offsetof(struct S, a),
                (int)__builtin_offsetof(struct S, b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 4");
}

TEST(Compiler, offsetofUnionIsZero) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U { int a; char b; };
        int main(void) {
            printf("%d %d", (int)__builtin_offsetof(union U, a),
                (int)__builtin_offsetof(union U, b));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, offsetofTypedefAndAnonymousStruct) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef struct { char a; char b; } T;
        int main(void) {
            printf("%d %d", (int)__builtin_offsetof(T, b),
                (int)__builtin_offsetof(struct { int x; int y; }, y));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, offsetofIsIce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { char a; int b; };
        static int off = __builtin_offsetof(struct S, b);
        enum { OFF = __builtin_offsetof(struct S, b) };
        int main(void) {
            int v = 4;
            switch (v) {
            case __builtin_offsetof(struct S, b):
                printf("%d %d", off, OFF);
                break;
            default:
                printf("0");
                break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

TEST(Compiler, offsetofMacroAndContainerOf) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define offsetof(type, member) __builtin_offsetof(type, member)
        #define container_of(ptr, type, member) \
            ((type *) ((char *)(ptr) - offsetof(type, member)))
        struct S { int a; int b; };
        int main(void) {
            struct S s;
            int *pb;
            s.a = 3;
            s.b = 9;
            pb = &s.b;
            printf("%d %d", (int)offsetof(struct S, b), container_of(pb, struct S, b)->a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 3");
}

} // namespace
