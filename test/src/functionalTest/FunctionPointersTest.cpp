#include "TestFixtures.h"

namespace {

TEST(Compiler, functionReturningPointer) {
    SourceProgram program{R"prg(
        int g;

        int *get(void) {
            return &g;
        }

        int main() {
            int *p;
            g = 5;
            p = get();
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, simpleFunctionPointer) {
    SourceProgram program{R"prg(
        int add1(int x) {
            return x + 1;
        }

        int main() {
            int (*fp)(int);
            fp = add1;
            printf("%d", fp(5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, functionPointerReassign) {
    SourceProgram program{R"prg(
        int f1(int x) {
            return x;
        }

        int f2(int x) {
            return x + 10;
        }

        int main() {
            int (*fp)(int);
            fp = f1;
            printf("%d ", fp(3));
            fp = f2;
            printf("%d", fp(3));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 13");
}


// Outer function pointer must not steal the abstract parameter's '*'.
TEST(Compiler, pointerReturnWithAbstractPointerParam) {
    SourceProgram program{R"prg(
        int x;
        int *getp(int *);
        int *getp(int *p) {
            return p;
        }
        int main() {
            x = 9;
            printf("%d", *getp(&x));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

// C typedef of function type; parameter decays to pointer (git: refs_for_each_cb fn).
TEST(Compiler, functionTypedefParameter) {
    SourceProgram program{R"prg(
        typedef int cb_fn(int x);

        int apply(cb_fn fn, int v) {
            return fn(v);
        }

        int inc(int x) {
            return x + 1;
        }

        int main() {
            printf("%d", apply(inc, 10));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}


// Function designator as global function-pointer initializer (git archive-tar:
// static void (*write_block)(const void *) = tar_write_block).
TEST(Compiler, globalFunctionPointerInitializer) {
    SourceProgram program{R"prg(
        static void say(void) {
            printf("ok");
        }
        static void (*fp)(void) = say;
        int main() {
            fp();
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// External function address in a multi-word global array (git git.c commands table:
// cmd_add etc. appear as dq operands and need `extern` for NASM).
TEST(Compiler, multiWordGlobalExternalFunctionAddresses) {
    SourceProgram program{R"prg(
        int strlen(const char *s);

        int (*table[])(const char *) = { strlen };

        int main() {
            printf("%d", table[0]("hello"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

// git-style command table: struct array with name string + external fn + flags.
TEST(Compiler, multiWordStructArrayExternalFunctionAddresses) {
    SourceProgram program{R"prg(
        int strlen(const char *s);

        struct Cmd {
            const char *name;
            int (*fn)(const char *);
            int flags;
        };

        static struct Cmd commands[] = {
            { "a", strlen, 9 },
            { "b", strlen, 1 }
        };

        int main() {
            printf("%s %d %d %s %d",
                commands[0].name,
                commands[0].fn("hi"),
                commands[0].flags,
                commands[1].name,
                commands[1].fn("x"));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("a 2 9 b 1");
}

// git get_builtin: walk a command table with `commands + i` (array-to-pointer
// decay + scaled pointer arithmetic), compare names, return the entry pointer.
TEST(Compiler, structArrayPlusIntegerLookup) {
    SourceProgram program{R"prg(
        int strcmp(const char *a, const char *b);

        struct Cmd {
            const char *name;
            int flags;
        };

        static struct Cmd commands[] = {
            { "add", 9 },
            { "version", 0 },
            { "write-tree", 1 }
        };

        static struct Cmd *get_builtin(const char *s) {
            int i;
            for (i = 0; i < 3; i = i + 1) {
                struct Cmd *p;
                p = commands + i;
                if (!strcmp(s, p->name))
                    return p;
            }
            return 0;
        }

        int main() {
            struct Cmd *p;
            p = get_builtin("version");
            if (!p) {
                printf("null");
                return 0;
            }
            printf("%s %d", p->name, p->flags);
            p = get_builtin("add");
            printf(" %s %d", p->name, p->flags);
            p = get_builtin("nope");
            if (!p)
                printf(" none");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("version 0 add 9 none");
}

// Explicit dereference of a function pointer (git reftable: (*reftable_malloc_ptr)(sz)).
TEST(Compiler, starredFunctionPointerCall) {
    SourceProgram program{R"prg(
        int inc(int x) {
            return x + 1;
        }
        int main() {
            int (*fp)(int);
            fp = inc;
            printf("%d", (*fp)(5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

// (*opt->callback)(...) form used by git parse-options.
TEST(Compiler, starredMemberFunctionPointerCall) {
    SourceProgram program{R"prg(
        struct Opt {
            int (*callback)(int);
        };
        int inc(int x) {
            return x + 1;
        }
        int main() {
            struct Opt opt;
            struct Opt *p;
            opt.callback = inc;
            p = &opt;
            printf("%d %d", (*opt.callback)(5), (*p->callback)(7));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6 8");
}

// Prototype with abstract function-typedef parameter, then definition with a name
// (git object-name: each_abbrev_fn, void * vs each_abbrev_fn fn, void *cb_data).
TEST(Compiler, functionTypedefAbstractPrototypeThenDefinition) {
    SourceProgram program{R"prg(
        typedef int each_fn(int);
        int apply(each_fn, int);
        int apply(each_fn fn, int v) {
            return fn(v);
        }
        int inc(int x) {
            return x + 1;
        }
        int main() {
            printf("%d", apply(inc, 10));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}



// &function_designator is the function address, not the address of a temp that
// holds it (git unit-test: OPT_STRING_LIST .callback = &parse_opt_string_list).
TEST(Compiler, addressOfFunctionDesignatorScalar) {
    SourceProgram program{R"prg(
        int inc(int x) {
            return x + 1;
        }
        int main() {
            int (*fp)(int);
            fp = &inc;
            printf("%d", fp(5));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

// Local designated init with &func (git parse-options tables on the stack).
TEST(Compiler, localDesignatedInitAddressOfFunction) {
    SourceProgram program{R"prg(
        int inc(int x) {
            return x + 1;
        }
        struct Opt {
            int type;
            int (*callback)(int);
        };
        int main() {
            struct Opt o = { .type = 1, .callback = &inc };
            printf("%d %d", o.type, o.callback(41));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 42");
}

// Same with incomplete local array of options (git unit-test options[]).
TEST(Compiler, localStructArrayDesignatedInitAddressOfFunction) {
    SourceProgram program{R"prg(
        int inc(int x) {
            return x + 1;
        }
        struct Opt {
            int type;
            int (*callback)(int);
        };
        int main() {
            struct Opt options[] = {
                { .type = 13, .callback = &inc },
                { .type = 0 }
            };
            printf("%d %d %d", options[0].type, options[0].callback(9), options[1].type);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("13 10 0");
}

// Indirect call with 8 args (2 on stack). After stack pushes, RSP-relative
// load of the callee pointer must account for the new RSP, or it reads a
// different local (git show_one_reflog_ent: call *fn becomes call *refname).
TEST(Compiler, indirectCallEightArgsStackCallee) {
    SourceProgram program{R"prg(
        int cb(int a, int b, int c, int d, int e, int f, int g, int h) {
            return a + b + c + d + e + f + g + h;
        }
        int invoke(int (*fn)(int, int, int, int, int, int, int, int), int h) {
            return fn(1, 2, 3, 4, 5, 6, 7, h);
        }
        int main() {
            printf("%d", invoke(cb, 8));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("36");
}

// Same pattern as git each_reflog_ent_fn: callback is a parameter, last two
// args are on the stack, and a string "name" is also a parameter.
TEST(Compiler, indirectCallEightArgsWithStringParam) {
    SourceProgram program{R"prg(
        int cb(const char *name, int b, int c, int d, int e, int f, int g, void *data) {
            (void)data;
            return name[0] + b + c + d + e + f + g;
        }
        int walk(const char *name, int (*fn)(const char *, int, int, int, int, int, int, void *), void *data) {
            return fn(name, 1, 2, 3, 4, 5, 6, data);
        }
        int main() {
            char n[2];
            n[0] = 'A';
            n[1] = 0;
            printf("%d", walk(n, cb, 0) - 'A');
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("21");
}

} // namespace
