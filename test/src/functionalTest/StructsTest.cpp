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
