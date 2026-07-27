#include "TestFixtures.h"

namespace {

// C11 anonymous union: nested members flatten into the enclosing struct.
TEST(Compiler, anonymousUnionMemberFlatten) {
    SourceProgram program{R"prg(
        struct S {
            int tag;
            union {
                int i;
                int j;
            };
        };

        int main() {
            struct S s;
            s.tag = 1;
            s.i = 42;
            printf("%d %d %d", s.tag, s.i, s.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 42 42");
}

// C11 anonymous struct inside a union.
TEST(Compiler, anonymousStructMemberFlatten) {
    SourceProgram program{R"prg(
        union U {
            int raw;
            struct {
                int lo;
                int hi;
            };
        };

        int main() {
            union U u;
            u.lo = 3;
            u.hi = 4;
            printf("%d %d", u.lo, u.hi);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}

} // namespace
