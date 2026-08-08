#include "TestFixtures.h"

namespace {

TEST(Compiler, anonymousUnionMemberFlatten) {
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, anonymousStructMemberFlatten) {
    SourceProgram program{R"prg(#include <stdio.h>
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

// Tagged nested type-only is not an anonymous member (no Outer.x).
TEST(Compiler, taggedNestedTypeOnlyDoesNotFlatten) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Outer {
            struct Inner {
                int x;
            };
            int z;
        };
        int main() {
            struct Outer o;
            o.z = 5;
            printf("%d %d", o.z, (int)sizeof(struct Outer));
            return 0;
        }
    )prg"};
    program.compile();
    // Only z (4 bytes); Inner tag is not a member.
    program.runAndExpect("5 4");
}

TEST(Compiler, anonymousUnionSizeAfterTag) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int tag;
            union {
                int i;
                int *p;
            };
        };
        int main() {
            printf("%d", (int)sizeof(struct S));
            return 0;
        }
    )prg"};
    program.compile();
    // int + padding + pointer = 16 on SysV x86-64
    program.runAndExpect("16");
}

} // namespace
