#include "TestFixtures.h"

namespace {


// Global pointer operand to * must load into scratch and use that register; must not
// call getAssignedRegister on the global home (no register cache on globals).
TEST(Compiler, readThroughGlobalPointer) {
    SourceProgram program{R"prg(
        int x;
        int *p;

        int main() {
            x = 3;
            p = &x;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


TEST(Compiler, writeThroughGlobalPointer) {
    SourceProgram program{R"prg(
        int x;
        int *p;

        int main() {
            x = 1;
            p = &x;
            *p = 9;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


TEST(Compiler, readWriteThroughGlobalPointerToGlobal) {
    SourceProgram program{R"prg(
        int g;
        int *p;

        int main() {
            g = 2;
            p = &g;
            *p = 4;
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}


TEST(Compiler, rmwThroughGlobalPointer) {
    SourceProgram program{R"prg(
        int x;
        int *p;

        int main() {
            x = 5;
            p = &x;
            *p = *p + 1;
            printf("%d", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}


// Static pointer to another global (git walker: **process_queue_end = &process_queue).
TEST(Compiler, globalAddressConstantInitializer) {
    SourceProgram program{R"prg(
        int g;
        int *p = &g;

        int main() {
            g = 7;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7");
}


// Pointer + integer is still an address constant (C 6.6). git OPT_INTEGER does
// .value = (v) + BARF_UNLESS_SIGNED(*(v)) which is &field + 0. Without folding
// that form, the static option table stores NULL and parse-options SEGVs on -U.
TEST(Compiler, globalAddressPlusZeroInitializer) {
    SourceProgram program{R"prg(
        int g;
        int *p = &g + 0;

        int main() {
            g = 11;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}


// Same through a designated struct field (git parse-options option tables).
TEST(Compiler, globalDesignatedAddressPlusZeroInitializer) {
    SourceProgram program{R"prg(
        int flag;
        struct Opt {
            void *value;
        };
        static struct Opt o = { .value = &flag + 0 };

        int main() {
            flag = 3;
            printf("%d", *(int *)o.value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}


// Member address + 0 (exact OPT_INTEGER / interactive_opts.context pattern).
TEST(Compiler, globalMemberAddressPlusZeroInitializer) {
    SourceProgram program{R"prg(
        struct Opts {
            int context;
            int interhunk;
        };
        static struct Opts opts = { -1, -1 };
        struct Opt {
            void *value;
        };
        static struct Opt table[] = {
            { .value = &opts.context + 0 },
            { .value = &opts.interhunk + 0 }
        };

        int main() {
            opts.context = 4;
            opts.interhunk = 2;
            printf("%d %d", *(int *)table[0].value, *(int *)table[1].value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4 2");
}


// Non-zero pointer element offset in a static initializer.
TEST(Compiler, globalAddressPlusOneInitializer) {
    SourceProgram program{R"prg(
        int arr[2];
        int *p = &arr[0] + 1;

        int main() {
            arr[0] = 1;
            arr[1] = 9;
            printf("%d", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


// Address of first array element as a global constant (git: *p = &arr[0]).
TEST(Compiler, globalAddressOfArrayElementZero) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };
        static struct S arr[3];
        static struct S *p = &arr[0];

        int main() {
            arr[0].x = 9;
            printf("%d", p->x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}


// Non-zero index is still an address constant: &arr[i] -> arr + i*sizeof(*arr)
// (gcc/nasm: .quad arr+N). Scalar global pointer init (git hash_algos[GIT_HASH_SHA1]).
TEST(Compiler, globalAddressOfArrayElementNonZero) {
    SourceProgram program{R"prg(
        struct S {
            int x;
            int y;
        };
        static struct S arr[3];
        static struct S *p = &arr[1];

        int main() {
            arr[1].x = 42;
            arr[1].y = 7;
            printf("%d %d", p->x, p->y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42 7");
}


// Designated struct field from &arr[i] (git: static repository repo =
// { .hash_algo = &hash_algos[GIT_HASH_SHA1] }). Must not silently zero.
TEST(Compiler, globalDesignatedAddressOfArrayElement) {
    SourceProgram program{R"prg(
        struct Algo {
            int id;
            int pad;
        };
        static struct Algo algos[3];
        static struct Repo {
            int unused;
            struct Algo *hash_algo;
        } repo = { .hash_algo = &algos[1] };

        int main() {
            algos[1].id = 11;
            printf("%d", repo.hash_algo->id);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}


TEST(Compiler, globalNullPointerCastInitializer) {
    SourceProgram program{R"prg(
        int *p = ((void *)0);

        int main() {
            printf("%d", p == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
