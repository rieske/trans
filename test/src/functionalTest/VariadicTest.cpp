#include "TestFixtures.h"

#include <cstdlib>
#include <fstream>
#include <string>

namespace {

// C23 va_start form (optional last-named) rewrites to the same SysV tag setup.
TEST(Compiler, c23VaStartReadsFirstVararg) {
    SourceProgram program{R"prg(
        int first_c23(int named, ...) {
            __builtin_va_list ap;
            __builtin_c23_va_start(ap, named);
            int first = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return first;
        }

        int main() {
            printf("%d", first_c23(7, 42));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("42");
}

// SysV AMD64 va_list is a 24-byte tag (or array-of-1 of that tag), not void*.
TEST(Compiler, vaListSizeIsTwentyFour) {
    SourceProgram program{R"prg(
        int main() {
            printf("%d", (int)sizeof(__builtin_va_list));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("24");
}

// Tag fields must be filled so that gp_offset skips named register args.
TEST(Compiler, vaStartGpOffsetSkipsNamedArg) {
    SourceProgram program{R"prg(
        int first_and_offset(int named, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, named);
            int gp = (int)ap[0].gp_offset;
            int first = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return gp * 1000 + first;
        }

        int main() {
            /* named in rdi => gp_offset starts at 8; first vararg is 42 */
            printf("%d", first_and_offset(7, 42));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("8042");
}

// Passing va_list to a wrapper must pass a pointer to the tag (array decay),
// matching libc / git strbuf_vaddf style helpers.
TEST(Compiler, vaListParameterDecaysToPointer) {
    SourceProgram program{R"prg(
        int take_ap(__builtin_va_list ap) {
            return __builtin_va_arg(ap, int);
        }
        int wrap(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, n);
            int v = take_ap(ap);
            __builtin_va_end(ap);
            return v;
        }
        int main() {
            printf("%d", wrap(0, 99));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("99");
}

// Linked binary must call real libc vsnprintf, not a trans_vsnprintf adapter.
TEST(Compiler, variadicUsesLibcVsnprintfSymbol) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            fmt_to(buf, 64, "%s", "ok");
            printf("%s", buf);
            return 0;
        }
    )prg", "va_libc_sym"};

    program.compileWithPreprocess();
    program.runAndExpect("ok");

    const std::string exe = program.getExecutablePath();
    const std::string nmOut = program.getSourceFilePath() + ".nm";
    remove(nmOut.c_str());
    ASSERT_EQ(system(("nm -u " + exe + " > " + nmOut + " 2>/dev/null").c_str()), 0);
    std::ifstream in(nmOut);
    std::string syms((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    EXPECT_THAT(syms, HasSubstr("vsnprintf"));
    EXPECT_THAT(syms, Not(HasSubstr("trans_vsnprintf")));
}

TEST(Compiler, variadicSumTwoExtraArgs) {
    SourceProgram program{R"prg(
        int add2(int base, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, base);
            int a = __builtin_va_arg(ap, int);
            int b = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return base + a + b;
        }

        int main() {
            printf("%d", add2(1, 2, 3));
            return 0;
        }
    )prg"};

    // Frontend rewrites __builtin_va_* to SysV tag operations.
    program.compileWithPreprocess();
    program.runAndExpect("6");
}

TEST(Compiler, variadicOneExtraArg) {
    SourceProgram program{R"prg(
        int first_extra(int x, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, x);
            int y = __builtin_va_arg(ap, int);
            __builtin_va_end(ap);
            return y;
        }

        int main() {
            printf("%d", first_extra(10, 42));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("42");
}

TEST(Compiler, variadicVaCopy) {
    SourceProgram program{R"prg(
        int sum_twice(int n, ...) {
            __builtin_va_list ap;
            __builtin_va_list cp;
            __builtin_va_start(ap, n);
            __builtin_va_copy(cp, ap);
            int a = __builtin_va_arg(ap, int);
            int b = __builtin_va_arg(cp, int);
            __builtin_va_end(ap);
            __builtin_va_end(cp);
            return a + b;
        }

        int main() {
            printf("%d", sum_twice(1, 21));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("42");
}

// SysV va_list tag must be acceptable to libc vsnprintf (git strbuf_vaddf).
TEST(Compiler, variadicVsnprintfThroughLibc) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            fmt_to(buf, 64, "%s/%s", "hello", "world");
            printf("%s", buf);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("hello/world");
}

TEST(Compiler, variadicVsnprintfIntsThroughLibc) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            fmt_to(buf, 64, "%d+%d=%d", 2, 3, 5);
            printf("%s", buf);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("2+3=5");
}

// Double through va_list -> vsnprintf (git strbuf_addf / json-writer fmt_double).
// Doubles live in the XMM portion of reg_save_area (fp_offset).
TEST(Compiler, variadicVsnprintfDoubleThroughLibc) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            double d;
            d = 3.14;
            fmt_to(buf, 64, "%.2f", d);
            printf("%s", buf);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("3.14");
}

// git trace2/tr2_tgt_normal.c fn_exit_fl / fn_atexit:
//   strbuf_addf(..., "exit elapsed:%.6f code:%d", elapsed, code);
// Double in xmm0, int in next free GP - pure SysV tag walk.
// code must be non-zero: dual-ABI float GP shadows put double bits in the int
// slot and low 32 bits of 1.5 are 0, so code:0 can pass spuriously.
TEST(Compiler, variadicVsnprintfDoubleThenIntThroughLibc) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            double elapsed;
            int code;
            elapsed = 1.5;
            code = 42;
            fmt_to(buf, 64, "exit elapsed:%.6f code:%d", elapsed, code);
            printf("%s", buf);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("exit elapsed:1.500000 code:42");
}

// Same shape as git child_exit: ints first, double last.
TEST(Compiler, variadicVsnprintfIntsThenDoubleThroughLibc) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char buf[64];
            double elapsed;
            elapsed = 2.25;
            fmt_to(buf, 64, "child_exit[%d] pid:%d code:%d elapsed:%.6f", 1, 2, 3, elapsed);
            printf("%s", buf);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("child_exit[1] pid:2 code:3 elapsed:2.250000");
}

// Direct libc printf with mixed double then int (non-variadic callee path).
TEST(Compiler, printfDoubleThenIntUsesSysV) {
    SourceProgram program{R"prg(
        int main() {
            double elapsed;
            int code;
            elapsed = 1.5;
            code = 42;
            printf("exit elapsed:%.6f code:%d", elapsed, code);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("exit elapsed:1.500000 code:42");
}

// Two named (non-format) args then double then int - strbuf_addf shape:
// void strbuf_addf(struct strbuf *sb, const char *fmt, ...)
// Caller must use pure SysV for every callee, not only the printf family.
TEST(Compiler, variadicTwoNamedThenDoubleThenInt) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void addf(char *storage, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(storage, 64, fmt, ap);
            __builtin_va_end(ap);
        }
        int main() {
            char storage[64];
            double elapsed;
            int code;
            elapsed = 1.5;
            code = 42;
            addf(storage, "exit elapsed:%.6f code:%d", elapsed, code);
            printf("%s", storage);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("exit elapsed:1.500000 code:42");
}

// git merge-ort path_msg: 8 named parameters before "...". Last named (fmt) is
// stack-passed. va_start must set overflow_arg_area to &fmt+8 (first variadic),
// not the first stack slot (which is still a named arg - often NULL).
// Bug symptom: "CONFLICT ((null)): Merge conflict in CONFLICT (%s): ..."
TEST(Compiler, variadicEightNamedThenTwoStrings) {
    SourceProgram program{R"prg(
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void path_msg(void *opt, int type, int hint,
                      const char *primary, const char *o1, const char *o2,
                      void *other_paths, const char *fmt, ...) {
            __builtin_va_list ap;
            char storage[96];
            __builtin_va_start(ap, fmt);
            vsnprintf(storage, 96, fmt, ap);
            __builtin_va_end(ap);
            printf("%s", storage);
        }
        int main() {
            path_msg(0, 1, 0, "numbers", 0, 0, 0,
                     "CONFLICT (%s): Merge conflict in %s", "content", "numbers");
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("CONFLICT (content): Merge conflict in numbers");
}

// More than 6 total args: the 7th+ are stack-passed via overflow_arg_area
// (git attr_check_initl with many attribute names + NULL).
TEST(Compiler, variadicNullTerminatedBeyondSixArgs) {
    SourceProgram program{R"prg(
        int count_until_null(const char *one, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, one);
            int n = 1;
            const char *p;
            while ((p = __builtin_va_arg(ap, const char *)) != 0) {
                n = n + 1;
            }
            __builtin_va_end(ap);
            return n;
        }
        int main() {
            printf("%d", count_until_null("a", "b", "c", "d", "e", "f", 0));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("6");
}

// Same pattern but verify the stack-passed string is readable (not just NULL).
TEST(Compiler, variadicStackPassedStringArg) {
    SourceProgram program{R"prg(
        int last_non_null(const char *one, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, one);
            const char *last = one;
            const char *p;
            while ((p = __builtin_va_arg(ap, const char *)) != 0) {
                last = p;
            }
            __builtin_va_end(ap);
            return last[0];
        }
        int main() {
            printf("%c", last_non_null("a", "b", "c", "d", "e", "f", "g", 0));
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("g");
}

// Nested variadic call before outer va_start must not clobber the outer
// function's save-area pointers. Regression for git run_commit_hook:
// strvec_pushf (variadic) runs before va_start; with a single TLS slot the
// outer va_start then walks the dead inner frame and mis-reads stack varargs
// (4 named + 4 varargs: prepare-commit-msg / commit-msg), crashing under
// MALLOC_PERTURB_ in t5326 "create new additional packs".
TEST(Compiler, variadicNestedCallPreservesOuterStackVarargs) {
    SourceProgram program{R"prg(
        void inner_fmt(const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            (void)__builtin_va_arg(ap, const char *);
            __builtin_va_end(ap);
        }
        /* 4 named GP args (like run_commit_hook), then NULL-terminated strings. */
        int outer_hooks(int a, const char *b, int *c, const char *name, ...) {
            char sink;
            (void)a;
            (void)b;
            (void)c;
            (void)sink;
            inner_fmt("%s", "nested");
            __builtin_va_list ap;
            __builtin_va_start(ap, name);
            int count = 0;
            const char *s;
            while ((s = __builtin_va_arg(ap, const char *)) != 0) {
                count = count + 1;
                sink = s[0];
            }
            __builtin_va_end(ap);
            return count;
        }
        int main() {
            int n = outer_hooks(1, "idx", 0, "prepare-commit-msg",
                                "editmsg", "arg1", "arg2", 0);
            printf("%d", n);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("3");
}

// Concurrent variadic calls must not share process-wide reg_save pointers.
// Regression for git multi-threaded index-pack: format_object_header ->
// xsnprintf races produced wrong object OIDs and pack idx validation errors.
TEST(Compiler, variadicConcurrentThreadsKeepDistinctArgs) {
    SourceProgram program{R"prg(
        #include <pthread.h>
        int vsnprintf(char *s, unsigned long n, const char *fmt, __builtin_va_list ap);
        void fmt_to(char *buf, unsigned long n, const char *fmt, ...) {
            __builtin_va_list ap;
            __builtin_va_start(ap, fmt);
            vsnprintf(buf, n, fmt, ap);
            __builtin_va_end(ap);
        }
        struct job {
            int id;
            int ok;
        };
        void *worker(void *arg) {
            struct job *j = (struct job *)arg;
            char buf[64];
            int i;
            j->ok = 1;
            for (i = 0; i < 2000; i = i + 1) {
                int a = j->id * 100000 + i;
                int b = 7 * j->id + (i & 255);
                fmt_to(buf, 64, "blob %d %d", a, b);
                /* Expected: "blob <a> <b>" - parse back and compare. */
                {
                    int ga = 0;
                    int gb = 0;
                    int k = 0;
                    /* skip "blob " */
                    while (buf[k] != 0 && buf[k] != ' ') k = k + 1;
                    if (buf[k] == ' ') k = k + 1;
                    while (buf[k] >= '0' && buf[k] <= '9') {
                        ga = ga * 10 + (buf[k] - '0');
                        k = k + 1;
                    }
                    if (buf[k] == ' ') k = k + 1;
                    while (buf[k] >= '0' && buf[k] <= '9') {
                        gb = gb * 10 + (buf[k] - '0');
                        k = k + 1;
                    }
                    if (ga != a || gb != b) {
                        j->ok = 0;
                        break;
                    }
                }
            }
            return 0;
        }
        int main() {
            pthread_t t[8];
            struct job jobs[8];
            int i;
            int bad = 0;
            for (i = 0; i < 8; i = i + 1) {
                jobs[i].id = i + 1;
                jobs[i].ok = 0;
                pthread_create(&t[i], 0, worker, &jobs[i]);
            }
            for (i = 0; i < 8; i = i + 1) {
                pthread_join(t[i], 0);
                if (jobs[i].ok == 0) bad = bad + 1;
            }
            printf("%d", bad);
            return 0;
        }
    )prg"};

    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// Product limit: multi-word va_arg is not a supported ABI surface (runtime value
// of the struct fields is wrong / garbage today). Pin that the frontend still
// accepts the syntax and the program links and runs without trapping. A correct
// implementation should change the oracle to print `11` (q.a) instead of `1`.
TEST(Compiler, vaArgOfStructIsAcceptedByFrontend) {
    SourceProgram program{R"prg(
        struct Pair {
            long a;
            long b;
        };
        long first_field(int n, ...) {
            __builtin_va_list ap;
            struct Pair p;
            __builtin_va_start(ap, n);
            p = __builtin_va_arg(ap, struct Pair);
            __builtin_va_end(ap);
            return p.a;
        }
        int main() {
            struct Pair q;
            q.a = 11;
            q.b = 22;
            (void)first_field(0, q);
            printf("%d", 1);
            return 0;
        }
    )prg", "va_arg_struct"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("1");
}

} // namespace
