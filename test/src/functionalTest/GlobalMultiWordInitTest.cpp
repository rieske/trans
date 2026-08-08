#include "TestFixtures.h"

namespace {

TEST(Compiler, globalStructBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int a;
            int b;
        };
        struct S g = { 1, 2 };
        int main() {
            printf("%d %d", g.a, g.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, globalStructPartialZeroFill) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { 5 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 0 0");
}

TEST(Compiler, globalStructDesignatedInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int a;
            int b;
            int c;
        };
        struct S g = { .b = 42 };
        int main() {
            printf("%d %d %d", g.a, g.b, g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 42 0");
}

TEST(Compiler, globalNestedDesignatedInitializer) {
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
        struct Rev g = {
            .pruning.flags.recursive = 1
        };
        int main() {
            if (g.pruning.orderfile != 0) return 1;
            if (g.pruning.reverse != 0) return 2;
            if (g.pruning.flags.recursive != 1) return 3;
            if (g.pruning.flags.dense != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, globalArrayBraceInitializer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int a[3] = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", a[0], a[1], a[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, globalFlatNestedStruct) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Inner {
            int a;
            int b;
        };
        struct Outer {
            struct Inner in;
            int w;
        };
        struct Outer g = { 1, 2, 3 };
        int main() {
            printf("%d %d %d", g.in.a, g.in.b, g.w);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

} // namespace
