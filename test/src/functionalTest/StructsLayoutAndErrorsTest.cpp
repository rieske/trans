#include "TestFixtures.h"

namespace {

TEST(Compiler, flexibleArrayMemberSizeofAndAccess) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int n;
            int data[];
        };
        int main() {
            int buf[4];
            struct S *p;
            p = (struct S *)buf;
            p->n = 1;
            p->data[0] = 7;
            printf("%d %d %d", sizeof(struct S), p->n, p->data[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1 7");
}

TEST(Compiler, flexibleArrayMemberMustBeLast) {
    SourceProgram program{R"prg(
        struct S {
            int data[];
            int n;
        };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("structure member has incomplete type");
}

TEST(Compiler, flexibleArrayMemberCannotBeOnlyMember) {
    SourceProgram program{R"prg(
        struct S {
            int data[];
        };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("structure member has incomplete type");
}

TEST(Compiler, unionRejectsFlexibleArrayMember) {
    SourceProgram program{R"prg(
        union U {
            int n;
            int data[];
        };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("union member has incomplete type");
}

TEST(Compiler, duplicateStructMemberIsError) {
    SourceProgram program{R"prg(
        struct S {
            int n;
            int n;
        };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("duplicate structure member name");
}

TEST(Compiler, constIntStructMemberIsComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            const int n;
        };
        struct S s;
        int main() {
            printf("%d", (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, intConstStructMemberIsComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int const n;
        };
        int main() {
            struct S s;
            printf("%d", (int)sizeof(s));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, volatileIntStructMemberIsComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            volatile int n;
        };
        int main() {
            struct S s;
            s.n = 9;
            printf("%d %d", s.n, (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 4");
}

TEST(Compiler, constCharPointerStructMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            const char *key;
            int n;
        };
        int main() {
            struct S s;
            s.key = "ab";
            s.n = 3;
            printf("%s %d %d", s.key, s.n, (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ab 3 16");
}

TEST(Compiler, opensslParamShapedStructCompletes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct ossl_param_st {
            const char *key;
            unsigned int data_type;
            void *data;
            unsigned long data_size;
            unsigned long return_size;
        };
        typedef struct ossl_param_st OSSL_PARAM;
        int main() {
            OSSL_PARAM p;
            p.key = "k";
            p.data_type = 1;
            p.data = 0;
            p.data_size = 2;
            p.return_size = 3;
            printf("%s %d %d %d %d", p.key, p.data_type, (int)p.data_size,
                    (int)p.return_size, (int)sizeof(OSSL_PARAM));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("k 1 2 3 40");
}

TEST(Compiler, constIntStructMemberBraceInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            const int n;
            int m;
        };
        int main() {
            struct S s = { 7, 8 };
            printf("%d %d", s.n, s.m);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}


} // namespace
