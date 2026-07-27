#include "TestFixtures.h"

namespace {

// C99 designated initializer (git strbuf-style: { .buf = ... }).
TEST(Compiler, structDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { .b = 7 };
            printf("%d %d %d", s.a, s.b, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 7 0");
}

// Nested designator path (git REV_INFO_INIT: .pruning.flags.recursive = 1).
TEST(Compiler, structNestedDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct Flags {
            int recursive;
            int dense;
        };
        struct Prune {
            void *orderfile;
            int reverse;
            struct Flags flags;
        };
        struct Rev {
            int remerge_diff;
            struct Prune pruning;
            int limited;
        };
        int main() {
            struct Rev r = {
                .remerge_diff = 0,
                .pruning.flags.recursive = 1,
                .limited = 0
            };
            if (r.pruning.orderfile != 0) return 1;
            if (r.pruning.reverse != 0) return 2;
            if (r.pruning.flags.recursive != 1) return 3;
            if (r.pruning.flags.dense != 0) return 4;
            if (r.remerge_diff != 0 || r.limited != 0) return 5;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Multiple nested designators sharing a parent must not wipe prior stores.
TEST(Compiler, structNestedDesignatedMultipleLeaves) {
    SourceProgram program{R"prg(
        struct Flags {
            int recursive;
            int dense;
        };
        struct Outer {
            int x;
            struct Flags flags;
            int y;
        };
        int main() {
            struct Outer o = {
                .flags.recursive = 3,
                .flags.dense = 5,
                .y = 9
            };
            if (o.x != 0) return 1;
            if (o.flags.recursive != 3) return 2;
            if (o.flags.dense != 5) return 3;
            if (o.y != 9) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, arrayDesignatedInitializer) {
    SourceProgram program{R"prg(
        int main() {
            int a[3] = { [1] = 9 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 9 0");
}

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

TEST(Compiler, structDesignatedMemberArrayBrace) {
    SourceProgram program{R"prg(
        struct object_id {
            unsigned char hash[4];
        };
        int main() {
            struct object_id desig = { .hash = { 5 } };
            printf("%d %d", (int)desig.hash[0], (int)desig.hash[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0");
}

TEST(Compiler, designatedUnknownMemberIsError) {
    SourceProgram program{R"prg(
        struct S {
            int a;
        };
        int main() {
            struct S s = { .nope = 1 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("designated");
}

// Non-constant array designator must error (not fall back to positional).
TEST(Compiler, arrayDesignatorNonConstantIsError) {
    SourceProgram program{R"prg(
        int main() {
            int i = 1;
            int a[3] = { [i] = 9 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("designated");
}

// sizeof(int) is an ICE; after SA visit, designator index folds.
TEST(Compiler, arrayDesignatorSizeofFolds) {
    SourceProgram program{R"prg(
        int main() {
            int a[16] = { [sizeof(int)] = 7 };
            printf("%d", a[sizeof(int)]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}

TEST(Compiler, memberThenArrayDesignator) {
    SourceProgram program{R"prg(
        struct S {
            int a[3];
            int z;
        };
        int main() {
            struct S s = { .a[1] = 9 };
            printf("%d %d %d %d", s.a[0], s.a[1], s.a[2], s.z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 9 0 0");
}

TEST(Compiler, arrayThenMemberDesignator) {
    SourceProgram program{R"prg(
        struct Cell {
            int x;
            int y;
        };
        int main() {
            struct Cell a[2] = { [1].x = 4, [1].y = 5 };
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 4 5");
}

// Multi-step designator then positional: resume after designated leaf (C current object).
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

// Designator to aggregate + scalar uses current-object fill of that aggregate.
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
    program.runAndExpect("5 7 0");
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
    program.runAndExpect("1 2 0 0");
}

// Anonymous member designators (same flatten as member access).
TEST(Compiler, anonymousMemberDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct Outer {
            struct {
                int x;
                int y;
            };
            int z;
        };
        int main() {
            struct Outer o = { .x = 1, 2, 3 };
            printf("%d %d %d", o.x, o.y, o.z);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, globalSubWordArrayPack) {
    SourceProgram program{R"prg(
        unsigned char h[4] = { 1, 2, 3, 4 };
        int main() {
            printf("%d %d %d %d", (int)h[0], (int)h[1], (int)h[2], (int)h[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, globalUnionExcessIsError) {
    SourceProgram program{R"prg(
        union U {
            int i;
            int j;
        };
        union U g = { 1, 2 };
        int main() {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

TEST(Compiler, designatorOverwriteLastWins) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        int main() {
            struct S s = { .a = 1, .a = 2, .b = 3 };
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3");
}

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

} // namespace
