#include "TestFixtures.h"

namespace {

// Product contracts for the git-shaped C subset. Compiler::preprocessCommand injects the
// defines; these tests pin the end-to-end observable behavior so a future
// change to trailing preprocess flags cannot silently drop them.

// Headers that gate GNU statement-expressions on
//   defined __GNUC__ && defined __STDC__ && __STDC__
// must take the portable path because the driver forces -D__STDC__=0.
TEST(ProductContracts, stdcForcedZeroSelectsPortableHeaderPath) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            #if defined __STDC__ && __STDC__
            printf("%d", 1);
            #else
            printf("%d", 0);
            #endif
            return 0;
        }
    )prg", "contract_stdc0"};
    program.compile();
    program.runAndExpect("0");
}

// curl typecheck wrappers use statement expressions; the driver injects
// -DCURL_DISABLE_TYPECHECK so the plain prototype form is selected.
TEST(ProductContracts, curlTypecheckDisabledByDefault) {
    SourceProgram program{R"prg(#include <stdio.h>
        #if !defined(CURL_DISABLE_TYPECHECK)
        #error "CURL_DISABLE_TYPECHECK must be defined by the driver"
        #endif
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "contract_curl"};
    program.compile();
    program.runAndExpect("1");
}

TEST(ProductContracts, constantPFoldsIntegerIce) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", __builtin_constant_p(42));
            return 0;
        }
    )prg", "contract_bcp"};
    program.compile();
    program.runAndExpect("1");
}

// Attributes are stripped so post-gcc -E output remains in the supported subset.
TEST(ProductContracts, attributesStrippedAroundDeclarations) {
    SourceProgram program{R"prg(#include <stdio.h>
        int helper(void) __attribute__((unused)) __attribute__((pure));
        int helper(void) { return 11; }
        int main() {
            printf("%d", helper());
            return 0;
        }
    )prg", "contract_attr"};
    program.compile();
    program.runAndExpect("11");
}

// GNU __int128 is a real 16-byte INTEGER pair (SysV two GPRs), not a long stand-in.
TEST(ProductContracts, int128IsSixteenBytes) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d", (int)sizeof(__int128));
            return 0;
        }
    )prg", "contract_int128"};
    program.compile();
    program.runAndExpect("16");
}

// BUILD_ASSERT_OR_ZERO / ARRAY_SIZE product path: negative sizeof(char[N]) folds
// so the assert contributes 0 and MOVE_ARRAY element size stays pointer-width.
TEST(ProductContracts, buildAssertOrZeroNegativeSizeofContributesZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int n;
            n = (int)(sizeof(char[1 - 2 * !0]) - 1);
            printf("%d %d", n, (int)sizeof(void *));
            return 0;
        }
    )prg", "contract_build_assert"};
    program.compile();
    program.runAndExpect("0 8");
}

// git SWAP: sizeof(a) + BUILD_ASSERT_OR_ZERO(sizeof(a)==sizeof(b)). Inner sizeof
// of equal struct types must fold so the char[N] bound is 1 (not a VLA/pointer).
TEST(ProductContracts, buildAssertOrZeroSizeofEqualStructsContributesZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S { long a; long b; long c; };
        int main() {
            int n;
            n = (int)(sizeof(struct S) + (sizeof(char[1 - 2 * !(sizeof(struct S) == sizeof(struct S))]) - 1));
            printf("%d", n);
            return 0;
        }
    )prg", "contract_build_assert_sizeof_eq"};
    program.compile();
    program.runAndExpect("24");
}

// git oidtree_lookup: klen += BUILD_ASSERT_OR_ZERO(offsetof(hash) < offsetof(algo)).
// Unfolded offsetof makes the char[N] a VLA/pointer and klen += 7, so QUICK
// loose lookup never matches (reflog write / fetch into corrupt repo).
TEST(ProductContracts, buildAssertOrZeroOffsetofOrderContributesZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct object_id {
            unsigned char hash[32];
            unsigned algo;
        };
        int main() {
            int n;
            n = (int)(sizeof(struct object_id)
                + (sizeof(char[1 - 2 * !(__builtin_offsetof(struct object_id, hash)
                    < __builtin_offsetof(struct object_id, algo))]) - 1));
            printf("%d", n);
            return 0;
        }
    )prg", "contract_build_assert_offsetof"};
    program.compile();
    program.runAndExpect("36");
}

// Assert-style macros expand __func__ / __FUNCTION__ / __PRETTY_FUNCTION__;
// SA lowers them to the current function name in rodata.
TEST(ProductContracts, funcIdentifiersAreCurrentFunctionName) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            const char *a;
            const char *b;
            const char *c;
            a = __func__;
            b = __FUNCTION__;
            c = __PRETTY_FUNCTION__;
            printf("[%s][%s][%s]", a, b, c);
            return 0;
        }
    )prg", "contract_func"};
    program.compile();
    program.runAndExpect("[main][main][main]");
}

// ISO rejects GNU statement expressions. Default -std=gnu accepts them.
TEST(ProductContracts, statementExpressionsAreRejected) {
    SourceProgram program{R"prg(int printf(const char *, ...);
int scanf(const char *, ...);
        int main() {
            int x;
            x = ({ int y; y = 1; y + 2; });
            printf("%d", x);
            return 0;
        }
    )prg", "contract_stmt_expr"};
    program.addCompilerArg("-std=c");
    program.compileWithArgs({
            "--no-preprocess",
            program.getSourceFilePath()
    });
    ASSERT_FALSE(program.isCompiled());
    program.assertCompilationErrors("unexpected token");
}

// Named bit-fields are accepted for header parsing; widths are ignored (ordinary
// members). Pin sizeof so a future packing implementation cannot silently shrink
// layout without updating this contract.
TEST(ProductContracts, bitfieldWidthsIgnoredForLayout) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Flags {
            unsigned a : 1;
            unsigned b : 2;
            int : 32;
            int c;
        };
        int main() {
            struct Flags f;
            f.a = 1;
            f.b = 3;
            f.c = 40;
            printf("%d %d %d %d", (int)sizeof(struct Flags), f.a, f.b, f.c);
            return 0;
        }
    )prg", "contract_bitfield"};
    program.compile();
    // Three full int/unsigned members; anonymous padding field is dropped.
    // b keeps 3 (no bit-width truncation to a 2-bit field).
    program.runAndExpect("12 1 3 40");
}


// sizeof "..." uses lexical array length (TypeQuery::sizeofStringLiteralTokenBytes),
// not sizeof(const char*). ARRAY_SIZE("x") style macros rely on this.
TEST(ProductContracts, sizeofStringLiteralIsArrayLength) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d", (int)sizeof("abcd"), (int)sizeof(""));
            return 0;
        }
    )prg", "contract_sizeof_str"};
    program.compile();
    // "abcd" is 5 bytes with NUL; "" is 1 byte NUL.
    program.runAndExpect("5 1");
}

// &((T*)0)->member folds to field offset (SA FieldPlan + null constant base).
TEST(ProductContracts, offsetofArrowFromNullFolds) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S { int a; int b; };
        int main() {
            printf("%d", (int)((long)&((struct S*)0)->b));
            return 0;
        }
    )prg", "contract_offsetof"};
    program.compile();
    program.runAndExpect("4");
}

} // namespace

// Permanent product assign: any pointer may be stored into any pointer dest
// (git-shaped C after host preprocess; not ISO conversion ranks).
TEST(ProductContracts, loosePointerAssignCompiles) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int x;
            int *ip;
            char *cp;
            void *vp;
            x = 7;
            ip = &x;
            cp = (char *)0;
            vp = ip;
            ip = vp;
            cp = (char *)ip;
            printf("%d", *ip);
            return 0;
        }
    )prg", "contract_loose_ptr"};
    program.compile();
    program.runAndExpect("7");
}
