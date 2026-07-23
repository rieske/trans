#include "TestFixtures.h"

namespace {

TEST(Compiler, enumBasicValues) {
    SourceProgram program{R"prg(
        int main() {
            enum Color { RED, GREEN, BLUE };
            printf("%d %d %d", RED, GREEN, BLUE);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

TEST(Compiler, enumExplicitValues) {
    SourceProgram program{R"prg(
        int main() {
            enum { A = 10, B, C = 20, D };
            printf("%d %d %d %d", A, B, C, D);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 11 20 21");
}

TEST(Compiler, enumVariable) {
    SourceProgram program{R"prg(
        int main() {
            enum Color { RED, GREEN, BLUE };
            enum Color c;
            c = GREEN;
            printf("%d", c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, enumInArithmetic) {
    SourceProgram program{R"prg(
        int main() {
            enum { ONE = 1, TWO = 2 };
            printf("%d", ONE + TWO);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, enumGlobal) {
    SourceProgram program{R"prg(
        enum Status { OK, ERR };
        int main() {
            printf("%d %d", OK, ERR);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, enumAsFunctionArgument) {
    SourceProgram program{R"prg(
        int f(int x) {
            return x + 1;
        }
        int main() {
            enum { N = 41 };
            printf("%d", f(N));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

// C99 trailing comma after last enumerator (common in system headers, e.g. idtype_t).
TEST(Compiler, enumTrailingComma) {
    SourceProgram program{R"prg(
        int main() {
            enum { A, B, C, };
            printf("%d %d %d", A, B, C);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1 2");
}

// System headers use aliases like _SC_IOV_MAX = _SC_UIO_MAXIOV.
TEST(Compiler, enumInitializerReferencesPriorEnumerator) {
    SourceProgram program{R"prg(
        int main() {
            enum {
                A = 7,
                B = A,
                C,
                D = B + 10
            };
            printf("%d %d %d %d", A, B, C, D);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 7 8 17");
}

TEST(Compiler, enumNamedTypeReference) {
    SourceProgram program{R"prg(
        enum Color { RED, GREEN };
        int main() {
            enum Color c;
            c = RED;
            printf("%d", c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, enumComparison) {
    SourceProgram program{R"prg(
        int main() {
            enum { NO, YES };
            int x;
            x = YES;
            if (x == YES) {
                printf("yes");
            } else {
                printf("no");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("yes");
}

// Nested enum as a struct member type (common in git: wt-status DIR_*, am MERGE_*, etc.).
// Enumerators must be in scope after the struct definition.
TEST(Compiler, enumAnonymousInStructMember) {
    SourceProgram program{R"prg(
        struct S {
            enum { A = 1, B = 2 } flags;
        };
        int main() {
            struct S s;
            s.flags = A | B;
            printf("%d %d %d", A, B, s.flags);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}

TEST(Compiler, enumNamedInStructMember) {
    SourceProgram program{R"prg(
        struct S {
            enum Color { RED, GREEN, BLUE } c;
        };
        int main() {
            struct S s;
            s.c = GREEN;
            printf("%d %d", RED, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

// Flag-style enumerators with shifts, as in wt-status.h / am.c.
TEST(Compiler, enumBitflagsInStructMember) {
    SourceProgram program{R"prg(
        struct Worktree {
            enum {
                DIR_SHOW_IGNORED = 1 << 0,
                DIR_SHOW_OTHER = 1 << 1,
                DIR_HIDE_EMPTY = 1 << 2
            } flags;
        };
        int main() {
            int f;
            f = DIR_SHOW_IGNORED | DIR_HIDE_EMPTY;
            printf("%d %d %d %d", DIR_SHOW_IGNORED, DIR_SHOW_OTHER, DIR_HIDE_EMPTY, f);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 4 5");
}

} // namespace
