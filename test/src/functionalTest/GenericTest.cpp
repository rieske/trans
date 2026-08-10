#include "TestFixtures.h"

namespace {

TEST(Compiler, genericSelectsIntAssociation) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic(0, int: 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericSelectsDefault) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic((char)1, int: 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, genericSelectsPointerAssociation) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            int *p;
            p = &x;
            printf("%d", _Generic(p, int *: 3, default: 4));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, genericDropsTopLevelConstOnControlling) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            const int n = 0;
            printf("%d", _Generic(n, int: 5, default: 6));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, genericDoesNotEvaluateLosingArm) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic(0, int: 7, default: 1 / 0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, genericPointerPointeeConstDoesNotMatchPlainPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            int *p;
            p = &x;
            printf("%d", _Generic(p, const int *: 1, int *: 2, default: 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, genericArrayControllingDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            printf("%d", _Generic(a, int *: 8, default: 9));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, genericMemberControllingIsTheStructNotItsAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        struct T { struct S s; };
        int main() {
            struct T t;
            t.s.x = 0;
            printf("%d", _Generic(t.s, struct S: 1, struct S *: 2, default: 3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericSelectedLvalueCanBeAssigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int x;
            x = 0;
            _Generic(0, int: x, default: x) = 7;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, genericSelectedMemberLvalueCanBeAssigned) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S s;
            s.x = 0;
            _Generic(0, int: s.x, default: s.x) = 4;
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, genericGitHasDirSepShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *strchr(const char *, int);
        static inline int git_has_dir_sep(const char *path) {
            return !!_Generic(0 ? (path) : (void *)1,
                    const void *: (const char *)(strchr(path, '/')),
                    default: strchr(path, '/'));
        }
        int main() {
            printf("%d", git_has_dir_sep("a/b") != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
