#include "TestFixtures.h"

namespace {

TEST(Compiler, builtinConstantPOfIntegerLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_constant_p(1));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfIntegerIce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_constant_p(1 + 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfEnumConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        enum { OPT = 10002 };
        int main(void) {
            printf("%d", __builtin_constant_p(OPT));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfNullPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_constant_p((void *)0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfFloatLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_constant_p(1.0));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfStringLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            printf("%d", __builtin_constant_p("hi"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, builtinConstantPOfVariable) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            x = 1;
            printf("%d", __builtin_constant_p(x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, builtinConstantPOfCall) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int id(int x) { return x; }
        int main(void) {
            printf("%d", __builtin_constant_p(id(1)));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, builtinConstantPDoesNotEvaluateArgument) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            x = 0;
            printf("%d %d", __builtin_constant_p(x++), x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Compiler, builtinConstantPIsIce) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int folded = __builtin_constant_p(3);
        enum { E = __builtin_constant_p(1 + 1) };
        int main(void) {
            int a[1 + __builtin_constant_p(1)];
            int x;
            x = 0;
            switch (__builtin_constant_p(x)) {
            case 0:
                printf("%d %d %d", folded, E, (int)(sizeof(a) / sizeof(a[0])));
                break;
            default:
                printf("x");
                break;
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 2");
}

TEST(Compiler, builtinConstantPAcceptsAnyType) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S { int x; };
        int main(void) {
            struct S s;
            s.x = 1;
            printf("%d %d", __builtin_constant_p(s), __builtin_constant_p(1.5f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 1");
}

TEST(Compiler, builtinConstantPCurlSetoptShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        #define CURLOPT_URL 10002
        #define curl_easy_setopt(handle, option, value) \
            __extension__({ \
                int ok; \
                ok = 1; \
                if (__builtin_constant_p(option)) { \
                    if ((option) == CURLOPT_URL) { \
                        if (!__builtin_types_compatible_p(typeof(value), char *) \
                                && !__builtin_types_compatible_p(typeof(value), char[])) \
                            ok = 0; \
                    } \
                } \
                (void)(handle); \
                ok; \
            })
        int main(void) {
            char *url;
            url = 0;
            (void)curl_easy_setopt(0, CURLOPT_URL, "http");
            printf("%d %d",
                curl_easy_setopt(0, CURLOPT_URL, url),
                __builtin_constant_p(CURLOPT_URL));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, builtinConstantPWrongArityIsError) {
    SourceProgram program{R"prg(
        int main(void) {
            return __builtin_constant_p();
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("wrong number of arguments to __builtin_constant_p");
}

TEST(Compiler, isoStdRejectsBuiltinConstantP) {
    SourceProgram program{R"prg(
        int main(void) {
            return __builtin_constant_p(1);
        }
    )prg", {"-std=c"}};
    program.compile();
    program.assertCompilationErrors("symbol `__builtin_constant_p` is not defined");
}

} // namespace
