#include "TestFixtures.h"

namespace {

TEST(Compiler, unionBasicOverlay) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };

        int main() {
            union U u;
            u.i = 42;
            printf("%d", u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unionSizeIsMaxMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int *p;
        };

        int main() {
            union U u;
            int x;
            x = 7;
            u.p = &x;
            printf("%d %d", *u.p, (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    // pointer is 8 bytes; sizeof union should be 8
    program.runAndExpect("7 8");
}

TEST(Compiler, unionPointerArrow) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            int i;
            int j;
        };

        int main() {
            union U u;
            union U *pu;
            pu = &u;
            pu->i = 9;
            printf("%d", u.j);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

// C11 anonymous union: nested members flatten into the enclosing struct (system headers / git).
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

// C11 anonymous struct inside a union (e.g. ucontext / fpregs layout).
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

// git reftable_writer_add_ref:
//   struct reftable_record rec = { .type = REF, .u = { .ref = *ref } };
// Zeroing unwritten union arms must not clobber the active multi-word member.
TEST(Compiler, unionDesignatedInitDoesNotClobberActiveMember) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct rec {
            char *name;
            unsigned long update;
            int value_type;
            unsigned char val1[32];
        };
        struct outer {
            unsigned char type;
            union {
                struct rec ref;
                char pad[144];
            } u;
        };
        void check(struct rec *ref) {
            struct outer rec = {
                .type = 1,
                .u = {
                    .ref = *ref
                },
            };
            printf("%u %s %lu %d %u",
                   rec.type,
                   rec.u.ref.name ? rec.u.ref.name : "(null)",
                   rec.u.ref.update,
                   rec.u.ref.value_type,
                   rec.u.ref.val1[0]);
        }
        int main() {
            struct rec r1[] = { {
                .name = (char *) "b",
                .update = 1,
                .value_type = 1,
                .val1 = { 1, 2, 3, 0 },
            } };
            check(&r1[0]);
            return 0;
        }
    )prg", "union_desig_clobber"};
    program.compile();
    program.runAndExpect("1 b 1 1 1");
}

// Smaller case: designated union arm with a pointer must survive sibling zeroing.
TEST(Compiler, unionDesignatedInitKeepsPointerArm) {
    SourceProgram program{R"prg(#include <stdio.h>
        union U {
            char *p;
            long pad;
        };
        int main() {
            union U u = { .p = (char *) "hi" };
            printf("%s", u.p ? u.p : "(null)");
            return 0;
        }
    )prg", "union_desig_ptr"};
    program.compile();
    program.runAndExpect("hi");
}

// System V memory return applies to large unions the same as large structs
// (SysV classify: complete records of MEMORY class).
TEST(Compiler, largeUnionReturnByValue) {
    SourceProgram program{R"prg(#include <stdio.h>
        union Big {
            unsigned long words[3];
            char bytes[24];
        };

        union Big make(void) {
            union Big u;
            u.words[0] = 1;
            u.words[1] = 2;
            u.words[2] = 3;
            return u;
        }

        int main() {
            union Big u;
            u = make();
            printf("%lu %lu %lu", u.words[0], u.words[1], u.words[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

// Tagged nested type-only is not an anonymous member (no Outer.x).
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
    )prg", "anon_union_size_tag"};
    program.compile();
    // int + padding + pointer = 16 on SysV x86-64
    program.runAndExpect("16");
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
    )prg", "union_brace_first"};
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
    )prg", "nested_empty_union_brace"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, constIntUnionMemberIsComplete) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        union U {
            const int n;
            int m;
        };
        int main() {
            union U u;
            u.m = 4;
            printf("%d %d", u.m, (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 4");
}

} // namespace
