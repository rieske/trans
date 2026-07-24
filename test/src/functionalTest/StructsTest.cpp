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

} // namespace
