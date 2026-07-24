#include "TestFixtures.h"

namespace {


TEST(Compiler, structMemberReadWrite) {
    SourceProgram program{R"prg(
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


TEST(Compiler, structPointerArrow) {
    SourceProgram program{R"prg(
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


TEST(Compiler, globalStructMembers) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

} // namespace
