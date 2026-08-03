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

TEST(Compiler, genericOnlyDefault) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic((char)1, default: 6));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, genericVolatileControllingDropsTopLevelQualifier) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            volatile int n;
            n = 0;
            printf("%d", _Generic(n, int: 5, default: 6));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, genericTypeofAssociationSelectsInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic(0, typeof(int): 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericTypeofExpressionAssociation) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char c;
            c = 0;
            printf("%d", _Generic(c, typeof(c): 3, int: 4, default: 5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, genericFunctionDesignatorDecaysToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int id(void) {
            return 9;
        }
        int main() {
            printf("%d", _Generic(id, int (*)(void): 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericFoldsInSwitchCase) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int s;
            s = 0;
            switch (1) {
            case _Generic(0, int: 1, default: 2):
                s = 9;
                break;
            default:
                s = 0;
                break;
            }
            printf("%d", s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, genericFoldsAsFileScopeInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = _Generic(0, int: 7, default: 0);
        int main() {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, genericFoldsAsArrayBound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[_Generic(0, int: 3, default: 1)];
            printf("%d", (int)sizeof(a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
}

TEST(Compiler, genericNestedSelection) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", _Generic(0, int: _Generic(0, int: 11, default: 12), default: 13));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, genericUnionControlling) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U { int x; };
        int main() {
            union U u;
            u.x = 0;
            printf("%d", _Generic(u, union U: 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericAssociationArrayBoundUsesSizeofIdentifier) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int n;
            printf("%d", _Generic((int (*)[4])0, int (*)[sizeof(n)]: 1, default: 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, genericFoldsAsTypeNameArrayBound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", (int)sizeof(int[_Generic(0, int: 3, default: 1)]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("12");
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
