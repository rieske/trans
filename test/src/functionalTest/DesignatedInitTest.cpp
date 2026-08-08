#include "TestFixtures.h"

namespace {

// Basic / nested / array designated initializers.

TEST(Compiler, structDesignatedInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int a[3] = { [1] = 9 };
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 9 0");
}

TEST(Compiler, structDesignatedMemberArrayBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, anonymousMemberDesignatedInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
        unsigned char h[4] = { 1, 2, 3, 4 };
        int main() {
            printf("%d %d %d %d", (int)h[0], (int)h[1], (int)h[2], (int)h[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}

TEST(Compiler, designatorOverwriteLastWins) {
    SourceProgram program{R"prg(#include <stdio.h>
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

TEST(Compiler, designatorToAggregateBraceList) {
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
            struct Outer o = { .in = { 1, 2 } };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 0");
}

TEST(Compiler, designatorToAggregateNestedDesignatedBrace) {
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
            struct Outer o = { .in = { .b = 7 } };
            printf("%d %d %d", o.in.a, o.in.b, o.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 7 0");
}

TEST(Compiler, arrayDesignatorToStructBrace) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Cell {
            int x;
            int y;
        };
        int main() {
            struct Cell a[2] = { [1] = { 4, 5 } };
            printf("%d %d %d %d", a[0].x, a[0].y, a[1].x, a[1].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 4 5");
}

TEST(Compiler, designatorToArrayBraceList) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int a[3];
        };
        int main() {
            struct S s = { .a = { 1, 2, 3 } };
            printf("%d %d %d", s.a[0], s.a[1], s.a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

// Global re-zero after partial nested brace (DataWordSink onUnwritten).

TEST(Compiler, globalPartialBraceRezerosSibling) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
        };
        struct Outer g = { 1, 2, .in = { 5 } };
        int main() {
            printf("%d %d", g.in.a, g.in.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0");
}

TEST(Compiler, localPartialBraceRezerosSibling) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
        };
        int main() {
            struct Outer o = { 1, 2, .in = { 5 } };
            printf("%d %d", o.in.a, o.in.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0");
}

// C last-wins: multiple designators into a union (not positional excess).

TEST(Compiler, designatorTypeMismatchIsError) {
    SourceProgram program{R"prg(
        struct Inner {
            int x;
        };
        struct Outer {
            int a;
        };
        int main() {
            struct Inner i;
            i.x = 1;
            struct Outer o = { .a = i };
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

// Nested designator into a union member of a struct (flatten path).


} // namespace
