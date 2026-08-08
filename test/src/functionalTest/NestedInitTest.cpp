#include "TestFixtures.h"

namespace {

TEST(Compiler, nestedStructBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner i;
            int x;
        };
        int main() {
            struct Outer o = { { 1, 2 }, 3 };
            printf("%d %d %d", o.i.a, o.i.b, o.x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, arrayBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3] = { 1, 2, 3 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, arrayBracePartialZeroFills) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3] = { 7 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 0 0");
}

TEST(Compiler, arrayOfStructBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S a[2] = { { 1, 2 }, { 3, 4 } };
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, structNestedArrayBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct object_id {
            unsigned char hash[4];
        };
        int main() {
            struct object_id one = { { 1 } };
            struct object_id multi = { { 1, 2, 3, 4 } };
            printf("%d %d %d %d %d %d",
                    (int)one.hash[0], (int)one.hash[1],
                    (int)multi.hash[0], (int)multi.hash[1],
                    (int)multi.hash[2], (int)multi.hash[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 1 2 3 4");
}

TEST(Compiler, arrayBraceExcessElementsIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a[2] = { 1, 2, 3 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in array initializer");
}

TEST(Compiler, nestedScalarBraceExcessIsError) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };
        int main() {
            struct S s = { { 1, 2 } };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in scalar initializer");
}

TEST(Compiler, topLevelNestedScalarBraceExcessIsError) {
    SourceProgram program{R"prg(
        int main() {
            int x = { { 1, 2 } };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess elements in scalar initializer");
}

TEST(Compiler, unionBraceInitializesFirstMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };
        int main() {
            union U u = { 7 };
            printf("%d %d", u.i, u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7");
}

TEST(Compiler, nestedEmptyUnionBraceZeros) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int i;
        };
        struct S {
            union U u;
        };
        int main() {
            struct S s = { { 0 } };
            printf("%d", s.u.i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

} // namespace
