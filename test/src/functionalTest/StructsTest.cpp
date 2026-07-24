#include "TestFixtures.h"

namespace {

TEST(Compiler, structMemberReadWrite) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

TEST(Compiler, anonymousStructLocal) {
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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
    SourceProgram program{R"prg(
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

TEST(Compiler, dualTypeWholeStructAssignIsError) {
    SourceProgram program{R"prg(
        struct Inner {
            int v;
            int w;
        };
        struct Outer {
            struct Inner in;
        };

        int main() {
            struct Outer o;
            struct Inner t;
            o.in.v = 5;
            o.in.w = 6;
            t = o.in;
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("dual-type aggregate");
}
} // namespace
