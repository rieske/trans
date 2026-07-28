#include "TestFixtures.h"

namespace {

// Designator path resume, excess, and keep-later-members pins.

TEST(Compiler, arrayDesignatedThenPositional) {
    SourceProgram program{R"prg(
        int main() {
            int a[4] = { [1] = 2, 3 };
            printf("%d %d %d %d", a[0], a[1], a[2], a[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 2 3 0");
}

TEST(Compiler, nestedDesignatorThenPositional) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { .in.a = 1, 2 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0");
}

TEST(Compiler, arrayOfArrayDesignatorThenPositional) {
    SourceProgram program{R"prg(
        int main() {
            int a[2][2] = { [0][0] = 1, 2 };
            printf("%d %d %d %d", a[0][0], a[0][1], a[1][0], a[1][1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0 0");
}

TEST(Compiler, memberArrayDesignatorThenPositional) {
    SourceProgram program{R"prg(
        struct S {
            int a[3];
            int z;
        };
        int main() {
            struct S s = { .a[1] = 9, 8 };
            printf("%d %d %d %d", s.a[0], s.a[1], s.a[2], s.z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 9 8 0");
}

// Non-brace designator value initializes one current-object leaf; next list
// elements resume after the designated aggregate (not into its remaining members).

TEST(Compiler, designatorToAggregateThenPositional) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { .in = 5, 7 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0 7");
}

TEST(Compiler, designatorToArrayRowThenPositional) {
    SourceProgram program{R"prg(
        int main() {
            int a[2][2] = { [0] = 1, 2 };
            printf("%d %d %d %d", a[0][0], a[0][1], a[1][0], a[1][1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0 2 0");
}

// Same rule for a union member whose first arm is aggregate.

TEST(Compiler, globalNestedDesignatorThenPositional) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        struct Outer g = { .in.a = 1, 2 };
        int main() {
            printf("%d %d %d", g.in.a, g.in.b, g.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0");
}

// After designator path fill is exhausted, leftovers are excess (not earlier holes).

TEST(Compiler, designatorThenExcessIsError) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { .b = 1, 2, 3 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

TEST(Compiler, designatorLastThenPositionalIsError) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { .c = 1, 2 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

TEST(Compiler, arrayDesignatorThenExcessIsError) {
    SourceProgram program{R"prg(
        int main() {
            int a[4] = { [2] = 5, 6, 7 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

TEST(Compiler, globalDesignatorThenExcessIsError) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { .b = 1, 2, 3 };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

// Still valid: designator then one positional into the next current-object slot.

TEST(Compiler, designatorMiddleThenOnePositional) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { .b = 1, 2 };
            printf("%d %d %d", s.a, s.b, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

// Later designator must not wipe earlier positional stores (path-fill zero-on-exhaust).

TEST(Compiler, positionalThenDesignatorKeepsLaterMembers) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { 1, 2, .a = 9 };
            printf("%d %d %d", s.a, s.b, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 2 0");
}

TEST(Compiler, nestedBraceThenDesignatorKeepsSibling) {
    SourceProgram program{R"prg(
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        int main() {
            struct Outer o = { { 1, 2 }, .in.a = 9 };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 2 0");
}

TEST(Compiler, arrayPositionalThenDesignatorKeepsRest) {
    SourceProgram program{R"prg(
        int main() {
            int a[4] = { 1, 2, 3, [0] = 9 };
            printf("%d %d %d %d", a[0], a[1], a[2], a[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 2 3 0");
}

TEST(Compiler, globalPositionalThenDesignatorKeepsLaterMembers) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { 1, 2, .a = 9 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 2 0");
}

// Designator value that is a brace list initializes the whole designated object.


} // namespace
