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

} // namespace

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
