#include "TestFixtures.h"

namespace {

TEST(Compiler, typesCompatibleSameInt) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", __builtin_types_compatible_p(int, int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, typesCompatibleIntVsPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", __builtin_types_compatible_p(int, int *));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, typesCompatibleIgnoresTopLevelConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", __builtin_types_compatible_p(const int, int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, typesCompatibleKeepsNestedConst) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d", __builtin_types_compatible_p(const int *, int *));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, typesCompatibleTypedefUnderlying) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef int num;
        int main() {
            printf("%d", __builtin_types_compatible_p(num, int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, typesCompatibleArrayVsAddressOfElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[3];
            printf("%d", __builtin_types_compatible_p(__typeof__(a), __typeof__(&(a)[0])));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, typesCompatiblePointerVsAddressOfElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int *p;
            p = 0;
            printf("%d", __builtin_types_compatible_p(__typeof__(p), __typeof__(&(p)[0])));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, typesCompatibleFunctionPointerTypeName) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            printf("%d %d",
                __builtin_types_compatible_p(int (*)(int, int), int (*)(int, int)),
                __builtin_types_compatible_p(int (*)(int, int), int (*)(int)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 0");
}

TEST(Compiler, typesCompatibleFoldsAsArrayBound) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            int a[1 + __builtin_types_compatible_p(int, int)];
            printf("%d", (int)sizeof(a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, arraySizeMacroOnLocalArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_AN_ARRAY(arr) \
    BUILD_ASSERT_OR_ZERO(!__builtin_types_compatible_p(__typeof__(arr), __typeof__(&(arr)[0])))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]) + BARF_UNLESS_AN_ARRAY(x))
        int main() {
            int a[3];
            printf("%d", (int)ARRAY_SIZE(a));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, typesCompatibleUnknownTypeofIsError) {
    SourceProgram program{R"prg(
        int main() {
            return __builtin_types_compatible_p(typeof(nope), int);
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("cannot determine type of typeof operand");
}

} // namespace
