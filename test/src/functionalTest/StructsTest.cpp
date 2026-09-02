#include "TestFixtures.h"

namespace {

TEST(Compiler, structMemberReadWrite) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s;
            s.x = 1;
            s.y = 2;
            printf("%d %d", s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, structPointerArrow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s;
            struct S* p;
            p = &s;
            p->x = 3;
            p->y = 4;
            printf("%d %d", p->x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, arrowFromArrayMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        struct O { struct S items[2]; };
        int main() {
            struct O o;
            o.items[0].x = 21;
            o.items[1].x = 22;
            o.items->x = 23;
            printf("%d %d", o.items->x, o.items[1].x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("23 22");
}

TEST(Compiler, arrowFrom2DRow) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S rows[2][2];
            rows[0][0].x = 31;
            rows[0][1].x = 32;
            rows[1][0].x = 33;
            rows[0]->x = 34;
            printf("%d %d %d", rows[0]->x, rows[0][1].x, rows[1][0].x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("34 32 33");
}

TEST(Compiler, arrowFromDerefPtrToArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S a[2];
            struct S (*p)[2];
            a[0].x = 41;
            a[1].x = 42;
            p = &a;
            (*p)->x = 43;
            printf("%d %d", (*p)->x, a[1].x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("43 42");
}

TEST(Compiler, arrowFromArrayOfPointers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S s0;
            struct S s1;
            struct S *a[2];
            s0.x = 1;
            s1.x = 2;
            a[0] = &s0;
            a[1] = &s1;
            a[0]->x = 3;
            printf("%d %d %d", a[0]->x, a[1]->x, s0.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 2 3");
}

TEST(Compiler, structArrayElementPassedByValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct object_id { unsigned char hash[4]; };
        int eq(struct object_id a, struct object_id b) {
            return a.hash[0] == b.hash[0] && a.hash[1] == b.hash[1];
        }
        int main() {
            struct object_id keys[2];
            struct object_id key;
            keys[0].hash[0] = 7;
            keys[0].hash[1] = 1;
            keys[1].hash[0] = 9;
            keys[1].hash[1] = 1;
            key.hash[0] = 7;
            key.hash[1] = 1;
            printf("%d %d", eq(keys[0], key), eq(keys[1], key));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, structArrayElementAssignAndFieldStore) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct item { int s; int u; };
        int main() {
            struct item list[3];
            list[0].s = 1;
            list[0].u = 10;
            list[1].s = 2;
            list[1].u = 20;
            list[2].s = 3;
            list[2].u = 30;
            list[0] = list[2];
            list[1].s = 4;
            printf("%d %d %d %d", list[0].s, list[0].u, list[1].s, list[2].s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 30 4 3");
}

TEST(Compiler, structMemberPassedByValueAndCopied) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Inner { int v; int w; };
        struct Outer { struct Inner in; };
        int sum(struct Inner s) {
            return s.v + s.w;
        }
        int main() {
            struct Outer o;
            struct Inner t;
            o.in.v = 5;
            o.in.w = 6;
            t = o.in;
            t.v = 8;
            printf("%d %d %d", sum(o.in), t.v, o.in.v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 8 5");
}

TEST(Compiler, postfixIncrementArrowMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main() {
            struct S a[2];
            struct S *p;
            a[0].x = 1;
            a[1].x = 2;
            p = a;
            printf("%d", (p++)->x);
            printf(" %d", p->x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, postfixIncrementIndex) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[2];
            int *p;
            a[0] = 3;
            a[1] = 4;
            p = a;
            printf("%d", p++[0]);
            printf(" %d", p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

TEST(Compiler, anonymousStructLocal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            struct {
                int a;
                int b;
            } s;
            s.a = 10;
            s.b = 20;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20");
}

TEST(Compiler, structThreeMembers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Point {
            int x;
            int y;
            int z;
        };

        int main() {
            struct Point p;
            p.x = 1;
            p.y = 2;
            p.z = 3;
            printf("%d %d %d", p.x, p.y, p.z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, structMemberViaNestedAccess) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Inner {
            int v;
        };
        struct Outer {
            struct Inner in;
            int w;
        };

        int main() {
            struct Outer o;
            o.in.v = 5;
            o.w = 6;
            printf("%d %d", o.in.v, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}


TEST(Compiler, structMemberAssignFromMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S a;
            struct S b;
            a.x = 7;
            a.y = 8;
            b.x = a.x;
            b.y = a.y;
            printf("%d %d", b.x, b.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

TEST(Compiler, structArrowThroughLocalPointerChain) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int n;
        };

        int main() {
            struct S s;
            struct S* p;
            struct S** pp;
            p = &s;
            pp = &p;
            (*pp)->n = 9;
            printf("%d", s.n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, structMemberInExpression) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s;
            s.x = 10;
            s.y = 3;
            printf("%d", s.x + s.y * 2);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("16");
}


TEST(Compiler, structSelfReferentialPointerMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Node {
            int val;
            struct Node *next;
        };

        int main() {
            struct Node a;
            struct Node b;
            a.val = 1;
            b.val = 2;
            a.next = &b;
            b.next = 0;
            printf("%d %d", a.val, a.next->val);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, structArrayElementMemberAccess) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S a[2];
            a[0].x = 1;
            a[0].y = 2;
            a[1].x = 3;
            a[1].y = 4;
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, structDotOnPointerIsSemanticError) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };

        int main() {
            struct S s;
            struct S *p;
            p = &s;
            p.x = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("non-structure");
}

TEST(Compiler, incompleteStructObjectIsError) {
    SourceProgram program{R"prg(
        struct S;
        int main() {
            struct S s;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("incomplete type");
}

TEST(Compiler, returnStructMemberByValue) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Inner { int v; int w; };
        struct Outer { struct Inner in; };
        struct Inner get(struct Outer o) {
            return o.in;
        }
        int main() {
            struct Outer o;
            struct Inner t;
            o.in.v = 5;
            o.in.w = 6;
            t = get(o);
            printf("%d %d", t.v, t.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}
TEST(Compiler, structUnknownMemberIsError) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };

        int main() {
            struct S s;
            s.y = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("no member");
}

TEST(Compiler, structArrowOnNonPointerIsError) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };

        int main() {
            struct S s;
            s->x = 1;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("not a pointer");
}

TEST(Compiler, structSizeofAndAddressOfMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S s;
            int *p;
            s.x = 3;
            s.y = 4;
            p = &s.x;
            printf("%d %d", sizeof(struct S), *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 3");
}

TEST(Compiler, structNestedMemberReadWrite) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            int z;
            struct Inner in;
        };

        int main() {
            struct Outer o;
            o.z = 1;
            o.in.a = 2;
            o.in.b = 3;
            printf("%d %d %d", o.z, o.in.a, o.in.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, structAssignWholeLocal) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        struct S {
            int x;
            int y;
        };

        int main() {
            struct S a;
            struct S b;
            a.x = 9;
            a.y = 8;
            b = a;
            printf("%d %d", b.x, b.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 8");
}

TEST(Compiler, structDerefAndStoreTwoWords) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            long a;
            long b;
        };
        int main() {
            struct S s;
            struct S t;
            struct S u;
            struct S *p;
            s.a = 1;
            s.b = 2;
            p = &s;
            t = *p;
            u.a = 3;
            u.b = 4;
            *p = u;
            printf("%ld %ld %ld %ld", t.a, t.b, s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, structDerefAssignThroughPointers) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            long a;
            long b;
        };
        int main() {
            struct S s;
            struct S u;
            struct S *p;
            struct S *q;
            s.a = 1;
            s.b = 2;
            u.a = 7;
            u.b = 8;
            p = &s;
            q = &u;
            *p = *q;
            printf("%ld %ld", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
}

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

TEST(Compiler, flexibleArrayMemberOmittedInitializer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int n;
            int data[];
        };
        struct S s = { 1 };
        int main() {
            struct S t = { 2 };
            printf("%d %d", s.n, t.n);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, flexibleArrayMemberStringInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct T {
            int n;
            char s[];
        };
        struct T t = { 1, "hi" };
        int main() {
            printf("%d %d %d %d", t.n, t.s[0], t.s[1], t.s[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 104 105 0");
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

TEST(Compiler, constIntLocalCompile) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            const int c = 3;
            int s;
            s = c;
            printf("%d", s);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
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
