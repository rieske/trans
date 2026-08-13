#include "TestFixtures.h"

namespace {

TEST(Compiler, bitFieldPackedSizeof) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        struct S { int a:31; int b:2; };
        struct C { char a:1; char b:1; };
        struct Z { int :0; int x:1; };
        struct U { int :7; int x:1; };
        struct M { int a:1; char c; };
        struct Mix { char a:1; int b:1; };
        struct CI { char a:1; int i; };
        union Unu { int a:3; int b:5; };
        int main() {
            printf("%d %d %d %d %d %d %d %d %d",
                    (int)sizeof(struct P),
                    (int)sizeof(struct S),
                    (int)sizeof(struct C),
                    (int)sizeof(struct Z),
                    (int)sizeof(struct U),
                    (int)sizeof(struct M),
                    (int)sizeof(struct Mix),
                    (int)sizeof(struct CI),
                    (int)sizeof(union Unu));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 8 1 4 4 4 4 8 4");
}

TEST(Compiler, bitFieldAssignAndRead) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        int main() {
            struct P p;
            p.a = 3;
            p.b = 5;
            printf("%d %d", p.a, p.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5");
}

TEST(Compiler, bitFieldSignedOneBitIsNegative) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x:1; };
        int main() {
            struct S s;
            s.x = 1;
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, bitFieldUnsignedOneBitIsOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { unsigned x:1; };
        int main() {
            struct S s;
            s.x = 1;
            printf("%d", (int)s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, bitFieldStoreDoesNotClobberNextObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct K {
            const char *key;
            int fd;
            unsigned initialized : 1;
        };
        struct K a = { "A", 0, 0 };
        struct K b = { "B", 0, 0 };
        int main() {
            a.initialized = 1;
            printf("%s %s %d", a.key, b.key, (int)a.initialized);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("A B 1");
}

TEST(Compiler, bitFieldStoreTruncates) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x:3; };
        int main() {
            struct S s;
            s.x = 9;
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, bitFieldIncrement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x:4; };
        int main() {
            struct S s;
            s.x = 3;
            printf("%d ", ++s.x);
            printf("%d ", s.x++);
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4 5");
}

TEST(Compiler, bitFieldThroughPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int a:4; int b:4; };
        int main() {
            struct S s;
            struct S *p;
            p = &s;
            p->a = 1;
            p->b = 2;
            printf("%d %d", s.a, p->b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, bitFieldMixedWithOrdinary) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct M { int a:1; char c; };
        int main() {
            struct M m;
            m.a = 1;
            m.c = 7;
            printf("%d %d %d", (int)sizeof(struct M), m.a, (int)m.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 -1 7");
}

TEST(Compiler, bitFieldBool) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct B { _Bool f:1; int x:3; };
        int main() {
            struct B b;
            b.f = 1;
            b.x = 3;
            printf("%d %d %d", (int)sizeof(struct B), (int)b.f, b.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 1 3");
}

TEST(Compiler, bitFieldLongLongPacksInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct L { long long a:40; int b:8; };
        int main() {
            struct L s;
            s.a = 1;
            s.b = 2;
            printf("%d %d %d", (int)sizeof(struct L), (int)s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1 2");
}

TEST(Compiler, bitFieldUnionOverlay) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U { int a:3; int b:5; };
        int main() {
            union U u;
            u.b = 7;
            printf("%d %d", (int)sizeof(union U), u.a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 -1");
}

TEST(Compiler, bitFieldBraceInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        int main() {
            struct P p = { 1, 2 };
            printf("%d %d", p.a, p.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, bitFieldDesignatedInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        int main() {
            struct P p = { .b = 2, .a = 1 };
            printf("%d %d", p.a, p.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, bitFieldDesignatedInitZerosUnwritten) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        int main() {
            struct P p = { .b = 2 };
            printf("%d %d", p.a, p.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 2");
}

TEST(Compiler, bitFieldDesignatedResume) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int :2; int b:5; };
        int main() {
            struct P p = { .a = 1, 2 };
            printf("%d %d", p.a, p.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, bitFieldNestedInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct I { int a:3; int b:5; };
        struct O { int pad; struct I i; };
        int main() {
            struct O o = { 0, { 1, 2 } };
            printf("%d %d %d", o.pad, o.i.a, o.i.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

TEST(Compiler, bitFieldNestedFileScopeInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct I { int a:3; int b:5; };
        struct O { int pad; struct I i; };
        struct O g = { 7, { 1, 2 } };
        int main() {
            printf("%d %d %d", g.pad, g.i.a, g.i.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 1 2");
}

TEST(Compiler, bitFieldStoreDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        int main() {
            int before;
            struct P p;
            int after;
            before = 11;
            after = 22;
            p.a = 3;
            p.b = 5;
            printf("%d %d %d %d", before, p.a, p.b, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 3 5 22");
}

TEST(Compiler, bitFieldShortStoreDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { short a:3; short b:5; };
        int main() {
            int before;
            struct S s;
            int after;
            before = 11;
            after = 22;
            s.a = 3;
            s.b = 5;
            printf("%d %d %d %d", before, (int)s.a, (int)s.b, after);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 3 5 22");
}

TEST(Compiler, bitFieldFileScopeInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct P { int a:3; int b:5; };
        struct P g = { 1, 2 };
        int main() {
            printf("%d %d", g.a, g.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

} // namespace
