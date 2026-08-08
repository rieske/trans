#include "TestFixtures.h"

namespace {

// C current-object: scalars continue into nested aggregate without braces.
TEST(Compiler, flatInitOfNestedStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { 1, 2, 3 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, flatInitOfNestedStructPartial) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { 1, 2 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0");
}

TEST(Compiler, flatInitArrayOfStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int x;
            int y;
        };
        int main() {
            struct S a[2] = { 1, 2, 3, 4 };
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

// After flat fill of Inner with 1,2, scalar brace { 9 } initializes Outer.w.
TEST(Compiler, flatInitThenNestedBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { 1, 2, { 9 } };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 9");
}

} // namespace
