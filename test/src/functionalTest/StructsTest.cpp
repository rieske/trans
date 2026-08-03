#include "TestFixtures.h"

namespace {


TEST(Compiler, structMemberReadWrite) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            p.x = 3;
            p.y = 4;
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

// Local `struct T x = p->m` is lowered to field inits; the member must be loaded
// (git reftable: struct reftable_block_source source = data->source).
TEST(Compiler, localStructInitFromMemberCopiesVtable) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Ops { int (*fn)(int); };
        struct Data { struct Ops source; int x; };
        int plus1(int n) { return n + 1; }
        int main() {
            struct Data d;
            d.source.fn = plus1;
            d.x = 6;
            {
                struct Ops source = d.source;
                printf("%d", source.fn(d.x));
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, localStructInitFromArrowMemberCopiesVtable) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Ops { int (*fn)(int); };
        struct Data { struct Ops source; int x; };
        int plus1(int n) { return n + 1; }
        int main() {
            struct Data d;
            struct Data *p;
            p = &d;
            d.source.fn = plus1;
            d.x = 6;
            {
                struct Ops source = p->source;
                printf("%d", source.fn(d.x));
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, structPointerArrow) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            struct Point *pp;
            pp = &p;
            pp->x = 7;
            pp->y = 8;
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 8");
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

TEST(Compiler, dotFromDerefPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; int y; };
        int main() {
            struct S s;
            struct S *p;
            s.x = 1;
            s.y = 2;
            p = &s;
            (*p).x = 11;
            (*p).y = 12;
            printf("%d %d %d %d", (*p).x, (*p).y, s.x, s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 12 11 12");
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


TEST(Compiler, globalStructMembers) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        struct Point g;

        int main() {
            g.x = 1;
            g.y = 2;
            printf("%d %d", g.x, g.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}


TEST(Compiler, structIntPointerMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Holder {
            int value;
            int *ptr;
        };

        int main() {
            int x;
            struct Holder h;
            x = 5;
            h.value = 1;
            h.ptr = &x;
            printf("%d %d", h.value, *h.ptr);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 5");
}


TEST(Compiler, structSelfPointerMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Node {
            int value;
            struct Node *next;
        };

        int main() {
            struct Node a;
            struct Node b;
            a.value = 1;
            b.value = 2;
            a.next = &b;
            b.next = &a;
            printf("%d %d", a.next->value, b.next->value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 1");
}


TEST(Compiler, structPassByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int sum(struct Point p) {
            return p.x + p.y;
        }

        int main() {
            struct Point p;
            p.x = 3;
            p.y = 4;
            printf("%d", sum(p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


TEST(Compiler, structPassByAddress) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int sum(struct Point *p) {
            return p->x + p->y;
        }

        int main() {
            struct Point p;
            p.x = 3;
            p.y = 4;
            printf("%d", sum(&p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


TEST(Compiler, structPassByAddressMiddleArg) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int mix(int a, struct Point *p, int b) {
            return a + p->x + p->y + b;
        }

        int main() {
            struct Point p;
            p.x = 10;
            p.y = 20;
            printf("%d", mix(1, &p, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("33");
}


TEST(Compiler, structPassByValueAfterInts) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int mix(int a, int b, struct Point p) {
            return a + b + p.x + p.y;
        }

        int main() {
            struct Point p;
            p.x = 10;
            p.y = 20;
            printf("%d", mix(1, 2, p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("33");
}


TEST(Compiler, structPassByValueBeforeInts) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int mix(struct Point p, int a, int b) {
            return p.x + p.y + a + b;
        }

        int main() {
            struct Point p;
            p.x = 10;
            p.y = 20;
            printf("%d", mix(p, 1, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("33");
}


TEST(Compiler, structPassByValueMutatesOnlyCopy) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        void bump(struct Point p) {
            p.x = 100;
            p.y = 200;
            return;
        }

        int main() {
            struct Point p;
            p.x = 1;
            p.y = 2;
            bump(p);
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}


TEST(Compiler, structPassByAddressMutatesOriginal) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        void bump(struct Point *p) {
            p->x = 100;
            p->y = 200;
            return;
        }

        int main() {
            struct Point p;
            p.x = 1;
            p.y = 2;
            bump(&p);
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("100 200");
}


TEST(Compiler, structAssignCopies) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point a;
            struct Point b;
            a.x = 5;
            a.y = 6;
            b = a;
            a.x = 0;
            a.y = 0;
            printf("%d %d", b.x, b.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 6");
}


TEST(Compiler, structNestedMemberViaPointerChain) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Node {
            int value;
            struct Node *next;
        };

        int main() {
            struct Node a;
            struct Node b;
            struct Node c;
            a.value = 1;
            b.value = 2;
            c.value = 3;
            a.next = &b;
            b.next = &c;
            c.next = 0;
            printf("%d", a.next->next->value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


TEST(Compiler, structThreeMembers) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Triple {
            int a;
            int b;
            int c;
        };

        int sum(struct Triple t) {
            return t.a + t.b + t.c;
        }

        int main() {
            struct Triple t;
            t.a = 1;
            t.b = 2;
            t.c = 3;
            printf("%d", sum(t));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}


// Type qualifiers on struct members must not leave terminals on the parse stack
// (that used to corrupt the struct tag name to '{').
TEST(Compiler, structConstMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            const int x;
            int y;
        };
        int main() {
            struct S s;
            s.y = 2;
            printf("%d", s.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}


TEST(Compiler, structConstPointerMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            const int *p;
        };
        int main() {
            int v;
            struct S s;
            v = 9;
            s.p = &v;
            printf("%d", *s.p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


TEST(Compiler, structPointerToConstQualified) {
    // const after *: void *const p (const pointer, not pointer to const)
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int *const p;
        };
        int main() {
            int v;
            struct S s;
            v = 4;
            s.p = &v;
            printf("%d", *s.p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}


TEST(Compiler, structVolatileMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            volatile int x;
        };
        int main() {
            struct S s;
            s.x = 5;
            printf("%d", s.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}


TEST(Compiler, structConstMemberArrow) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            const int x;
            int y;
        };
        int read_y(struct S *s) {
            return s->y;
        }
        int main() {
            struct S s;
            s.y = 11;
            printf("%d", read_y(&s));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}


// Three-word `unsigned short int` in a struct member must stay unsigned.
// combineSpecQualifierTypeSpecs packages `short int` as one TypeSpecifier;
// without peeling that primitive when applying outer `unsigned`, loads use
// movsx and git similarity_index prints negative percents (p->score).
TEST(Compiler, unsignedShortIntStructMemberZeroExtends) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct pair {
            unsigned short int score;
        };
        int similarity(struct pair *p) {
            return p->score * 100 / 60000;
        }
        int main() {
            struct pair p;
            p.score = 36600;
            printf("%d %d", p.score, similarity(&p));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("36600 61");
}

// Nested member address: a[i].m and s.a.b modes (AddressBaseMode PointerValue + Lvalue name).
TEST(Compiler, nestedMemberAndArrayFieldAddress) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner { int v; };
        struct Outer { struct Inner a[2]; int z; };
        int main() {
            struct Outer o;
            o.a[0].v = 10;
            o.a[1].v = 32;
            o.z = 0;
            printf("%d", o.a[0].v + o.a[1].v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
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
