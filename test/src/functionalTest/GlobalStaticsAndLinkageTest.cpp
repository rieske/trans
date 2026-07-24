#include "TestFixtures.h"

namespace {



// Function-scope static must have static storage duration: survives return and
// is shared across calls (git hex.c: static char hexbuffer[][] / static int bufno).
TEST(Compiler, functionScopeStaticIntPersists) {
    SourceProgram program{R"prg(
        int next(void) {
            static int n;
            n = n + 1;
            return n;
        }
        int main() {
            printf("%d %d %d", next(), next(), next());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3");
}


// Returning a pointer to a function-scope static buffer must remain valid after
// the call (git hash_to_hex_algop returns static hexbuffer[bufno]).
TEST(Compiler, functionScopeStaticBufferReturn) {
    SourceProgram program{R"prg(
        char *getbuf(void) {
            static char buf[8];
            static int n;
            n = n + 1;
            buf[0] = 65;
            buf[1] = 48 + n;
            buf[2] = 0;
            return buf;
        }
        int main() {
            char *p;
            p = getbuf();
            printf("%s ", p);
            p = getbuf();
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("A1 A2");
}


// Static local with explicit constant initializer.
TEST(Compiler, functionScopeStaticWithInitializer) {
    SourceProgram program{R"prg(
        int bump(void) {
            static int n = 10;
            n = n + 1;
            return n;
        }
        int main() {
            printf("%d %d", bump(), bump());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11 12");
}


// Cast of address-of in a static designated init must keep the address constant
// (git setup.c / commit-graph: OPT_STRING(..., (void *)&opts.obj_dir)).
// Without peeling TypeCast, .value is emitted as 0 and get_arg writes through NULL.
TEST(Compiler, castAddressOfInStaticDesignatedInit) {
    SourceProgram program{R"prg(
        struct opt {
            int type;
            const char *long_name;
            void *value;
        };
        static char *obj_dir;
        static struct opt common_opts[] = {
            { .type = 4, .long_name = "object-dir", .value = (void *)&obj_dir },
            { .type = 0 },
        };
        int main() {
            if (common_opts[0].value == 0) {
                printf("nil");
                return 1;
            }
            if (common_opts[0].value != (void *)&obj_dir) {
                printf("bad");
                return 2;
            }
            printf("ok %d %s", common_opts[0].type, common_opts[0].long_name);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok 4 object-dir");
}


// Nested casts of &global (git sometimes uses (void *)(char *)&x).
TEST(Compiler, nestedCastAddressOfInStaticDesignatedInit) {
    SourceProgram program{R"prg(
        struct opt {
            void *value;
        };
        static int flag;
        static struct opt o = { .value = (void *)(int *)&flag };
        int main() {
            int *p;
            p = (int *)o.value;
            *p = 7;
            printf("%d", flag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


// Address of a struct member in a static designated init (git commit-graph:
// OPT_STRING(..., &opts.obj_dir) and OPT_BOOL(..., &opts.shallow)).
// Without member address constants, .value is 0 and get_arg writes through NULL.
TEST(Compiler, memberAddressInStaticDesignatedInit) {
    SourceProgram program{R"prg(
        struct opts {
            const char *obj_dir;
            int shallow;
        };
        static struct opts opts;
        struct option {
            int type;
            const char *long_name;
            void *value;
        };
        static struct option common_opts[] = {
            { .type = 4, .long_name = "object-dir", .value = &opts.obj_dir },
            { .type = 1, .long_name = "shallow", .value = &opts.shallow },
            { .type = 0 },
        };
        int main() {
            if (common_opts[0].value == 0 || common_opts[1].value == 0) {
                printf("nil");
                return 1;
            }
            *(const char **)common_opts[0].value = "objects";
            *(int *)common_opts[1].value = 1;
            printf("%s %d", opts.obj_dir, opts.shallow);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("objects 1");
}


// Non-zero member offset: second field must get label+N, not label.
TEST(Compiler, memberAddressNonZeroOffsetInStaticInit) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        static struct S s;
        static int *pb = &s.b;
        int main() {
            *pb = 42;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 42");
}


// Address of function-scope statics in a static aggregate initializer must use
// the mangled .data symbol (git builtin/commit.c: OPT_BOOL(..., &no_renames)).
// Without this, the option table relocates against bare names that never exist.
TEST(Compiler, functionScopeStaticAddressInStaticAggregate) {
    SourceProgram program{R"prg(
        struct opt {
            char *name;
            void *val;
        };
        int use(void) {
            static int no_renames = -1;
            static char *rename_score_arg = (char *)-1;
            static struct opt opts[2] = {
                { "no-renames", &no_renames },
                { "find-renames", &rename_score_arg },
            };
            int *p;
            char **q;
            p = (int *)opts[0].val;
            q = (char **)opts[1].val;
            printf("%d %d", *p, *q != 0);
            return 0;
        }
        int main() {
            use();
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1 1");
}


// Scalar pointer to a sibling function-scope static (same linkage rule).
TEST(Compiler, functionScopeStaticAddressScalarInit) {
    SourceProgram program{R"prg(
        int use(void) {
            static int x = 5;
            static int *p = &x;
            *p = *p + 1;
            return x;
        }
        int main() {
            printf("%d %d", use(), use());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 7");
}


// Address-of function in a static aggregate must emit the real function symbol,
// not the designator temporary (git OPT_STRVEC: .callback = &parse_opt_strvec).
TEST(Compiler, functionAddressInStaticAggregate) {
    SourceProgram program{R"prg(
        int cb(int x) {
            return x + 1;
        }
        struct opt {
            int (*callback)(int);
        };
        int use(void) {
            static struct opt o = { &cb };
            return o.callback(41);
        }
        int main() {
            printf("%d", use());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}


// NASM treats "strict" as a size-specifier keyword; C globals with that name
// must still assemble and link (git index-pack: static int strict).
TEST(Compiler, nasmReservedGlobalNameStrict) {
    SourceProgram program{R"prg(
        int strict;
        int main() {
            strict = 3;
            if (strict) {
                printf("%d", strict);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


// NASM instruction mnemonic "prefetch" must not break a C global of that name
// (git fetch: static int prefetch).
TEST(Compiler, nasmReservedGlobalNamePrefetch) {
    SourceProgram program{R"prg(
        int prefetch;
        int main() {
            prefetch = 9;
            printf("%d", prefetch);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


// Pure extern must not define storage (would shadow libc and multi-define across TUs).
// environ is provided by the C library.
TEST(Compiler, externVariableUsesLibcSymbol) {
    SourceProgram program{R"prg(
        extern char **environ;
        int main() {
            printf("%d", environ != 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}


// .data symbols that follow odd-length string constants must still be
// naturally aligned. Without `align` between db and dq, a long can land at
// e.g. 0x...043; pthread_mutex_t / cond in git grep then SIGABRT in futex.
TEST(Compiler, globalLongAfterStringConstantIsEightByteAligned) {
    SourceProgram program{R"prg(
        static long needs_align;
        int main() {
            printf("%d", (int)(((unsigned long)&needs_align) & 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}


// Multi-word global (pthread_mutex_t-sized) after short string storage must
// also be 8-byte aligned for glibc/futex (git builtin/grep.c: grep_mutex).
TEST(Compiler, multiWordGlobalAfterStringIsEightByteAligned) {
    SourceProgram program{R"prg(
        struct M {
            long a;
            long b;
            long c;
            long d;
            long e;
        };
        static char *msg = "xy";
        static struct M mutex_like;
        int main() {
            printf("%d", (int)(((unsigned long)&mutex_like) & 7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}


// git run-command.c:prep_childenv declares `extern char **environ` at block scope.
// That must link to the libc symbol, not allocate a stack local (would SEGV in strchr(*p)).
TEST(Compiler, blockScopeExternVariableUsesLibcSymbol) {
    SourceProgram program{R"prg(
        int count_env(void) {
            extern char **environ;
            const char *const *p;
            int n;
            n = 0;
            for (p = (const char *const *) environ; p && *p; p++) {
                n = n + 1;
                if (n > 3) {
                    break;
                }
            }
            return n;
        }
        int main() {
            printf("%d", count_env() > 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}


// static functions are local to the TU (no global export) but still callable.
TEST(Compiler, staticFunctionIsCallable) {
    SourceProgram program{R"prg(
        static int helper(void) {
            return 9;
        }
        int main() {
            printf("%d", helper());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


// git refs.c: extern T name[N]; then T name[] = { designated inits }.
// Redeclaration must be accepted (compatible incomplete/complete arrays).
TEST(Compiler, externSizedArrayThenIncompleteDefinition) {
    SourceProgram program{R"prg(
        enum E { A, B, C };
        struct Info {
            int x;
        };
        extern struct Info arr[C];
        struct Info arr[] = {
            [A] = { .x = 1 },
            [B] = { .x = 2 },
        };
        int main() {
            return arr[0].x + arr[1].x;
        }
    )prg"};
    program.compileWithPreprocess();
}


// File-scope fixed array brace init must store values (not only zero storage).
TEST(Compiler, globalIntArrayBraceInitializer) {
    SourceProgram program{R"prg(
        static int settings[3] = { 10, 20, 30 };
        int main() {
            printf("%d %d %d", settings[0], settings[1], settings[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20 30");
}


// File-scope char arrays must pack one byte per element (not one qword each).
// Required for git sane_ctype[256] and isalpha/iskeychar.
TEST(Compiler, globalCharArrayBraceInitializerPacked) {
    SourceProgram program{R"prg(
        unsigned char tbl[8] = { 1, 2, 3, 4, 5, 6, 7, 128 };
        int main() {
            printf("%d %d %d %d %d %d %d %d %d",
                (int)sizeof(tbl),
                tbl[0], tbl[1], tbl[2], tbl[3],
                tbl[4], tbl[5], tbl[6], tbl[7]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1 2 3 4 5 6 7 128");
}


// Designated indices into a packed global char table (git ctype.h sane_ctype).
TEST(Compiler, globalCharArrayDesignatedInitializerPacked) {
    SourceProgram program{R"prg(
        unsigned char tbl[16] = {
            [1] = 16,
            [2] = 32,
            [9] = 64,
            [15] = 128,
        };
        int main() {
            printf("%d %d %d %d %d %d %d",
                tbl[0], tbl[1], tbl[2], tbl[8], tbl[9], tbl[14], tbl[15]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 16 32 0 64 0 128");
}


// Short arrays also pack (stride 2); two elements share a qword.
TEST(Compiler, globalShortArrayBraceInitializerPacked) {
    SourceProgram program{R"prg(
        unsigned short tbl[4] = { 1, 2, 3, 0x8000 };
        int main() {
            printf("%d %d %d %d %d",
                (int)sizeof(tbl), tbl[0], tbl[1], tbl[2], tbl[3]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 1 2 3 32768");
}


// Incomplete array size comes from the brace initializer (git: static T a[] = {...}).
TEST(Compiler, globalIncompleteArrayBraceInitializerSize) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };
        static struct S settings[] = {
            { 1, 2 },
            { 3, 4 },
            { 5, 6 },
        };
        int main() {
            printf("%d %d %d %d %d %d %d",
                (int)(sizeof(settings) / sizeof(settings[0])),
                settings[0].x, settings[0].y,
                settings[1].x, settings[1].y,
                settings[2].x, settings[2].y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 1 2 3 4 5 6");
}


// File-scope array of fixed char rows initialized from string literals
// (git advice.c: static char advice_colors[][COLOR_MAXLEN] = { GIT_COLOR_RESET, ... }).
// Without packing each string into its row, *color is always 0 and color.advice is a no-op.
TEST(Compiler, globalCharArray2DStringInitializer) {
    SourceProgram program{R"prg(
        static char colors[][8] = {
            "\033[m",
            "\033[33m",
        };
        int main() {
            printf("%d %d %02x %02x %02x %02x %02x %s",
                (int)sizeof(colors),
                (int)(sizeof(colors) / 8),
                (int)(unsigned char)colors[0][0],
                (int)(unsigned char)colors[0][1],
                (int)(unsigned char)colors[1][0],
                (int)(unsigned char)colors[1][1],
                (int)(unsigned char)colors[1][2],
                colors[1][0] ? "on" : "off");
            return 0;
        }
    )prg"};
    program.compile();
    // 2 rows * 8; ESC '[' 'm'; ESC '[' '3' ...
    program.runAndExpect("16 2 1b 5b 1b 5b 33 on");
}


// Designated array indices + nested designated members (git tr2_sysenv_settings).
TEST(Compiler, globalStructArrayDesignatedInitializer) {
    SourceProgram program{R"prg(
        enum E { A = 0, B, C, MUST_BE_LAST };
        struct Entry {
            const char *name;
            const char *cfg;
            char *value;
            int flag;
        };
        static struct Entry settings[] = {
            [A] = { "e0", "c0" },
            [B] = { "e1", "c1" },
            [C] = { "e2", "c2" },
        };
        int main() {
            int n = (int)(sizeof(settings) / sizeof(settings[0]));
            printf("%d %d %s %s %s %s",
                n, MUST_BE_LAST,
                settings[0].name, settings[0].cfg,
                settings[2].name, settings[2].cfg);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 3 e0 c0 e2 c2");
}

} // namespace
