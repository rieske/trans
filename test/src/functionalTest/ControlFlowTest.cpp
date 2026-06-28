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

// TODO(gap): empty for-init `for (; cond; incr)` - no AST creator for
// `<iteration_stat_matched> ::= for ( ; <exp> ; <exp> ) <matched>`. Need grammar
// action / ContextualSyntaxNodeBuilder support for omitted for-init (valid C).
/*
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
*/

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

// TODO(gap): `break` / `continue` - no AST creator for `<jump_stat> ::= break ;`
// / `continue ;` (JumpStatement only logs "not implemented"; codegen throws).
// Need parse->AST, loop context (exit / continue labels) in semantic analysis,
// and Jump quadruples to those labels in CodeGeneratingVisitor.
/*
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
*/

} // namespace
