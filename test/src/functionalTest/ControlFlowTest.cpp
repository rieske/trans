#include "TestFixtures.h"

namespace {

TEST(Compiler, ifElseReturnsValue) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            if (a) {
                printf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0");
    program.runAndExpect("1", "1");
    program.runAndExpect("-3", "1");
}

TEST(Compiler, ifNested) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            int b;
            scanf("%ld %ld", &a, &b);
            if (a) {
                if (b) {
                    printf("%d", 3);
                } else {
                    printf("%d", 2);
                }
            } else {
                printf("%d", 1);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0", "1");
    program.runAndExpect("0 1", "1");
    program.runAndExpect("1 0", "2");
    program.runAndExpect("1 1", "3");
}

TEST(Compiler, ifEmptyThenBranch) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            if (a) {
            }
            printf("%d", a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0", "0");
    program.runAndExpect("7", "7");
}

TEST(Compiler, whileNeverRuns) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 0;
            while (n) {
                printf("%d", 9);
            }
            printf("%d", 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, whileSingleIteration) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = 1;
            while (n) {
                printf("%d", n);
                n = 0;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, nestedWhile) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int j;
            int sum;
            sum = 0;
            i = 0;
            while (i < 2) {
                j = 0;
                while (j < 2) {
                    sum = sum + 1;
                    j = j + 1;
                }
                i = i + 1;
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, forInsideWhile) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int j;
            int sum;
            sum = 0;
            i = 0;
            while (i < 2) {
                for (j = 0; j < 2; j = j + 1) {
                    sum = sum + 1;
                }
                i = i + 1;
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, forEmptyBody) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            for (i = 0; i < 3; i = i + 1) {
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forEmptyInit) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (; i < 3; i = i + 1) {
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forWithPointerInUpdate) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int* p;
            i = 0;
            p = &i;
            for (; i < 3; ++(*p)) {
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forNoIncrement) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            for (i = 0; i < 3; ) {
                i = i + 1;
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forNoInitNoIncrement) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (; i < 3; ) {
                i = i + 1;
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forNoClause) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            for (i = 0; ; i = i + 1) {
                if (i == 3) {
                    printf("%d", i);
                    return 0;
                }
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forNoClauseNoIncrement) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            for (i = 0; ; ) {
                if (i == 3) {
                    printf("%d", i);
                    return 0;
                }
                i = i + 1;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forNoInitNoClause) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (; ; i = i + 1) {
                if (i == 3) {
                    printf("%d", i);
                    return 0;
                }
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, forEmpty) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            for (; ; ) {
                if (i == 3) {
                    printf("%d", i);
                    return 0;
                }
                i = i + 1;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, earlyReturnSkipsCode) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            if (a) {
                return 0;
            }
            printf("%d", 9);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1", "");
    program.runAndExpect("0", "9");
}

TEST(Compiler, breakExitsWhile) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            i = 0;
            while (1) {
                i = i + 1;
                break;
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, continueSkipsIteration) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int sum;
            sum = 0;
            i = 0;
            while (i < 3) {
                i = i + 1;
                continue;
                sum = sum + 1;
            }
            printf("%d %d", i, sum);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 0");
}

TEST(Compiler, breakExitsFor) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            for (i = 0; i < 10; i = i + 1) {
                if (i == 3) {
                    break;
                }
            }
            printf("%d", i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, continueInForSkipsIncrementBody) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int sum;
            sum = 0;
            for (i = 0; i < 5; i = i + 1) {
                if (i == 2) {
                    continue;
                }
                sum = sum + i;
            }
            printf("%d", sum);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}


TEST(Compiler, nestedBreakOnlyExitsInner) {
    SourceProgram program{R"prg(
        int main() {
            int i;
            int j;
            int n;
            n = 0;
            i = 0;
            while (i < 2) {
                j = 0;
                while (1) {
                    j = j + 1;
                    break;
                }
                n = n + j;
                i = i + 1;
            }
            printf("%d %d", n, i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 2");
}

// Null statement `;` as a block item (common in macros / generated code).

// size_t overflow checks use `b > (~0UL - a)`. Signed jg treats ~0 as -1 and
// falsely reports overflow for small b (git st_add / unsigned_add_overflows).
TEST(Compiler, unsignedRelationalNearMax) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long a;
            unsigned long b;
            unsigned long maxv;
            maxv = 0xffffffffffffffffUL;
            a = 0;
            b = 10;
            if (b > maxv - a) {
                printf("overflow");
            } else {
                printf("ok %lu", a + b);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok 10");
}

// Same check as git unsigned_add_overflows(size, 1): int literal on the left,
// sizeof/UL-derived unsigned max on the right. Both sides must promote so the
// compare uses JA not JG (otherwise xmallocz(21) dies "Data too large").
TEST(Compiler, unsignedAddOverflowsMacroPattern) {
    SourceProgram program{R"prg(
        typedef unsigned long size_t;
        int main() {
            size_t size;
            size = 21;
            if (1 > (0xffffffffffffffffUL >> (8 * sizeof(unsigned long) - 8 * sizeof(size))) - size) {
                printf("overflow");
            } else {
                printf("ok");
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// High-bit unsigned still greater than zero (signed would say -1 < 0).
TEST(Compiler, unsignedGreaterThanZero) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long x;
            x = 0xffffffffffffffffUL;
            if (x > 0) {
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

// git strbuf_grow: if (!sb->alloc) ... then use sb->len / sb->buf after the join.
// Register-passed formals must still be available after a conditional branch
// when only one side spilled them (spill before conditional jump).
TEST(Compiler, formalLiveAcrossConditionalBranch) {
    SourceProgram program{R"prg(
        struct S {
            unsigned long alloc;
            unsigned long len;
            int *buf;
        };
        int use(struct S *sb, unsigned long extra) {
            int new_buf;
            unsigned long need;
            new_buf = !sb->alloc;
            need = sb->len + extra + 1;
            if (new_buf) {
                sb->buf = 0;
            }
            if (need > sb->alloc) {
                sb->alloc = need;
            }
            return new_buf;
        }
        int main() {
            struct S s;
            int r;
            s.alloc = 0;
            s.len = 0;
            s.buf = 0;
            r = use(&s, 10);
            printf("%d %lu", r, s.alloc);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 11");
}


} // namespace
