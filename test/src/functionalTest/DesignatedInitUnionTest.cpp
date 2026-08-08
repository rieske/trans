#include "TestFixtures.h"

namespace {

// Union designated init, residual zero, first-arm current-object.

TEST(Compiler, designatorToUnionAggregateThenPositional) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a[2];
            int b;
        };
        struct S {
            union U u;
            int z;
        };
        int main() {
            struct S s = { .u = 7, 9 };
            if (s.u.a[0] != 7) return 1;
            if (s.u.a[1] != 0) return 2;
            if (s.z != 9) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Anonymous member designators (same flatten as member access).

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

TEST(Compiler, unionMultiDesignatorLastWins) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };
        int main() {
            union U u = { .i = 1, .j = 2 };
            printf("%d %d", u.i, u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2");
}

TEST(Compiler, globalUnionMultiDesignatorLastWins) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };
        union U g = { .i = 1, .j = 2 };
        int main() {
            printf("%d %d", g.i, g.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2");
}

// Positional after a designator is still excess on a union.

TEST(Compiler, unionDesignatorThenPositionalIsError) {
    SourceProgram program{R"prg(
        union U {
            int i;
            int j;
        };
        int main() {
            union U u = { .i = 1, 2 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

// Brace-init zeros the whole union; active arm may be smaller than the object.

TEST(Compiler, localUnionDesignatorZerosResidualBytes) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            long l;
        };
        int main() {
            union U u = { .c = 1 };
            if (u.c != 1) return 1;
            if (u.l != 1) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Large union (> 8 bytes): FieldPlanSink must zero all words, not one register store.

TEST(Compiler, localLargeUnionDesignatorFullZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int a[4];
        };
        int main() {
            union U u = { .c = 7 };
            if ((int)u.c != 7) return 1;
            if (u.a[1] != 0) return 2;
            if (u.a[2] != 0) return 3;
            if (u.a[3] != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, globalLargeUnionDesignatorFullZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int a[4];
        };
        union U g = { .c = 7 };
        int main() {
            if ((int)g.c != 7) return 1;
            if (g.a[1] != 0) return 2;
            if (g.a[3] != 0) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Structure-to-scalar in a designator must be a semantic error (not half-lowered).

TEST(Compiler, designatorIntoStructUnionMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };
        struct S {
            int pad;
            union U u;
        };
        int main() {
            struct S s = { .u.j = 9 };
            printf("%d %d %d", s.pad, s.u.i, s.u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 9 9");
}

// After designating into a nested array arm, do not resume into sibling union arms.
// Leftover elements at root union are excess (not another member).

TEST(Compiler, unionNestedArrayDesignatorThenExcessIsError) {
    SourceProgram program{R"prg(
        union U {
            int a[2];
            int b;
        };
        int main() {
            union U u = { .a[0] = 1, 2, 3 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

TEST(Compiler, unionNestedArrayDesignatorThenPositional) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a[2];
            int b;
        };
        int main() {
            union U u = { .a[0] = 1, 2 };
            if (u.a[0] != 1) return 1;
            if (u.a[1] != 2) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Path fill through a union arm must resume into the enclosing struct, not the other arm.

TEST(Compiler, nestedUnionArrayDesignatorResumesOuterStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a[2];
            int b;
        };
        struct S {
            union U u;
            int z;
        };
        int main() {
            struct S s = { .u.a[0] = 1, 2, 3 };
            if (s.u.a[0] != 1) return 1;
            if (s.u.a[1] != 2) return 2;
            if (s.z != 3) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// C current-object: multi-element list fills aggregate first arm (no nested braces required).

TEST(Compiler, unionFirstArmArrayFlatFill) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a[2];
            int b;
        };
        int main() {
            union U u = { 1, 2 };
            if (u.a[0] != 1) return 1;
            if (u.a[1] != 2) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, unionFirstArmStructFlatFill) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Pair {
            int x;
            int y;
        };
        union U {
            struct Pair p;
            int b;
        };
        int main() {
            union U u = { 3, 4 };
            if (u.p.x != 3) return 1;
            if (u.p.y != 4) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Designator to whole union member with flat multi-element brace list for that arm.

TEST(Compiler, designatorToUnionMemberFlatFill) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a[2];
            int b;
        };
        struct S {
            union U u;
            int z;
        };
        int main() {
            struct S s = { .u = { 1, 2 }, .z = 9 };
            if (s.u.a[0] != 1) return 1;
            if (s.u.a[1] != 2) return 2;
            if (s.z != 9) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Still excess once the first arm is full.

TEST(Compiler, unionFirstArmFlatFillThenExcessIsError) {
    SourceProgram program{R"prg(
        union U {
            int a[2];
            int b;
        };
        int main() {
            union U u = { 1, 2, 3 };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("excess");
}

// Root union list: nested braces are a complete first-arm initializer (via first-arm stream).

TEST(Compiler, unionFirstArmNestedDesignatorBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Pair {
            int x;
            int y;
        };
        union U {
            struct Pair p;
            int b;
        };
        int main() {
            union U u = { { .y = 5 } };
            if (u.p.x != 0) return 1;
            if (u.p.y != 5) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Root union list: designator inside first-arm braces cannot name another arm.

TEST(Compiler, unionNestedBraceOtherArmIsError) {
    SourceProgram program{R"prg(
        struct Pair {
            int x;
        };
        union U {
            struct Pair p;
            int b;
        };
        int main() {
            union U u = { { .b = 2 } };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("designated");
}

// fillFromStream(union) with braces re-walks the union (arm designators apply).

TEST(Compiler, nestedUnionOuterWithInnerArmDesignator) {
    SourceProgram program{R"prg(#include <stdio.h>
        union Inner {
            int a;
            int b;
        };
        union Outer {
            union Inner i;
            long c;
        };
        int main() {
            union Outer o = { { .b = 2 } };
            if (o.i.b != 2) return 1;
            if (o.i.a != 2) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// After a prior designator, a brace list that fills a union member is whole-union init.

TEST(Compiler, positionalBraceUnionAfterDesignator) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a;
            int b;
        };
        struct S {
            int pad;
            union U u;
        };
        int main() {
            struct S s = { .pad = 1, { .b = 5 } };
            if (s.pad != 1) return 1;
            if (s.u.b != 5) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Designator value for a whole union still allows arm designators inside braces.

TEST(Compiler, designatorToUnionWithInnerArmDesignator) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int a;
            int b;
        };
        struct S {
            union U u;
            int z;
        };
        int main() {
            struct S s = { .u = { .b = 4 }, .z = 1 };
            if (s.u.b != 4) return 1;
            if (s.z != 1) return 2;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// Nested union filled via stream/scalar must still full-zero residual bytes.

TEST(Compiler, nestedUnionScalarInitZerosResidual) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int a[4];
        };
        struct S {
            union U u;
        };
        int main() {
            struct S s = { 7 };
            if ((int)s.u.c != 7) return 1;
            if (s.u.a[1] != 0) return 2;
            if (s.u.a[3] != 0) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, nestedUnionDesignatorScalarZerosResidual) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int a[4];
        };
        struct S {
            int pad;
            union U u;
        };
        int main() {
            struct S s = { .u = 7 };
            if (s.pad != 0) return 1;
            if ((int)s.u.c != 7) return 2;
            if (s.u.a[1] != 0) return 3;
            if (s.u.a[2] != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, globalNestedUnionScalarInitZerosResidual) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char c;
            int a[4];
        };
        struct S {
            union U u;
        };
        struct S g = { 7 };
        int main() {
            if ((int)g.u.c != 7) return 1;
            if (g.u.a[1] != 0) return 2;
            if (g.u.a[3] != 0) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


} // namespace
