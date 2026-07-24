#include "TestFixtures.h"

namespace {

// Product contracts for the git-shaped C subset. HostToolchain injects the
// defines; these tests pin the end-to-end observable behavior so a future
// change to trailing preprocess flags cannot silently drop them.

// Headers that gate GNU statement-expressions on
//   defined __GNUC__ && defined __STDC__ && __STDC__
// must take the portable path because the driver forces -D__STDC__=0.
TEST(ProductContracts, stdcForcedZeroSelectsPortableHeaderPath) {
    SourceProgram program{R"prg(
        int main() {
            #if defined __STDC__ && __STDC__
            printf("%d", 1);
            #else
            printf("%d", 0);
            #endif
            return 0;
        }
    )prg", "contract_stdc0"};
    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// curl typecheck wrappers use statement expressions; the driver injects
// -DCURL_DISABLE_TYPECHECK so the plain prototype form is selected.
TEST(ProductContracts, curlTypecheckDisabledByDefault) {
    SourceProgram program{R"prg(
        #if !defined(CURL_DISABLE_TYPECHECK)
        #error "CURL_DISABLE_TYPECHECK must be defined by the driver"
        #endif
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "contract_curl"};
    program.compileWithPreprocess();
    program.runAndExpect("1");
}

// Product approximation: __builtin_types_compatible_p → 0 (ARRAY_SIZE path).
TEST(ProductContracts, typesCompatiblePAlwaysZero) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", __builtin_types_compatible_p(int, int));
            return 0;
        }
    )prg", "contract_tcp"};
    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// Product approximation: __builtin_constant_p → 0.
TEST(ProductContracts, constantPAlwaysZero) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", __builtin_constant_p(42));
            return 0;
        }
    )prg", "contract_bcp"};
    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// Attributes are stripped so post-gcc -E output remains in the supported subset.
TEST(ProductContracts, attributesStrippedAroundDeclarations) {
    SourceProgram program{R"prg(
        int helper(void) __attribute__((unused)) __attribute__((pure));
        int helper(void) { return 11; }
        int main() {
            printf("%d", helper());
            return 0;
        }
    )prg", "contract_attr"};
    program.compileWithPreprocess();
    program.runAndExpect("11");
}

// Product approximation: _Generic always selects the default association when
// present (type matching is not implemented). A "fix" that picks int: would
// break headers that rely on this stand-in.
TEST(ProductContracts, genericAlwaysPicksDefaultWhenPresent) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 0;
            printf("%d", _Generic(x, int: 1, default: 2));
            return 0;
        }
    )prg", "contract_generic"};
    program.compileWithPreprocess();
    program.runAndExpect("2");
}

// Product stand-in: __int128 is treated as long long (8 bytes), not a true 16-byte type.
TEST(ProductContracts, int128IsEightByteStandIn) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", (int)sizeof(__int128));
            return 0;
        }
    )prg", "contract_int128"};
    program.compileWithPreprocess();
    program.runAndExpect("8");
}

// BUILD_ASSERT_OR_ZERO / ARRAY_SIZE product path: negative sizeof(char[N]) folds
// so the assert contributes 0 and MOVE_ARRAY element size stays pointer-width.
TEST(ProductContracts, buildAssertOrZeroNegativeSizeofContributesZero) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            n = (int)(sizeof(char[1 - 2 * !0]) - 1);
            printf("%d %d", n, (int)sizeof(void *));
            return 0;
        }
    )prg", "contract_build_assert"};
    program.compileWithPreprocess();
    program.runAndExpect("0 8");
}

// Assert-style macros expand __func__ / __FUNCTION__ / __PRETTY_FUNCTION__;
// product maps them to empty string literals.
TEST(ProductContracts, funcIdentifiersMapToEmptyString) {
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    program.runAndExpect("[][][]");
}

// GNU statement expressions remain unsupported (headers must take portable paths).
TEST(ProductContracts, statementExpressionsAreRejected) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = ({ int y; y = 1; y + 2; });
            printf("%d", x);
            return 0;
        }
    )prg", "contract_stmt_expr"};
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
    SourceProgram program{R"prg(
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
    program.compileWithPreprocess();
    // Three full int/unsigned members; anonymous padding field is dropped.
    // b keeps 3 (no bit-width truncation to a 2-bit field).
    program.runAndExpect("12 1 3 40");
}


// sizeof "..." uses lexical array length (TypeQuery::sizeofStringLiteralTokenBytes),
// not sizeof(const char*). ARRAY_SIZE("x") style macros rely on this.
TEST(ProductContracts, sizeofStringLiteralIsArrayLength) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d %d", (int)sizeof("abcd"), (int)sizeof(""));
            return 0;
        }
    )prg", "contract_sizeof_str"};
    program.compile();
    // "abcd" is 5 bytes with NUL; "" is 1 byte NUL.
    program.runAndExpect("5 1");
}

// &((T*)0)->member folds to field offset (product_approx::foldOffsetofArrowFromNull).
TEST(ProductContracts, offsetofArrowFromNullFolds) {
    SourceProgram program{R"prg(
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
