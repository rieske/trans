#include "TestFixtures.h"

namespace {

// Member array bounds are ICEs. sizeof in that bound is an ICE once the
// operand type is known, including a prior complete array or an incomplete
// array completed by its initializer.

TEST(Compiler, sizeofStructMemberBoundFromCompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[6];
        struct generated_pack {
            void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
        };
        int main(void) {
            struct generated_pack pack;
            pack.tempfiles[0] = (void *)(long)1;
            pack.tempfiles[5] = (void *)(long)6;
            printf("%d %d %d",
                (int)sizeof(struct generated_pack),
                (int)(long)pack.tempfiles[0],
                (int)(long)pack.tempfiles[5]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 1 6");
}

TEST(Compiler, sizeofStructMemberBoundFromIncompleteArrayInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static struct {
            const char *name;
            unsigned optional:1;
        } exts[] = {
            {".pack"},
            {".rev", 1},
            {".mtimes", 1},
            {".bitmap", 1},
            {".promisor", 1},
            {".idx"},
        };
        struct generated_pack {
            void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
        };
        int main(void) {
            struct generated_pack *pack;
            pack = 0;
            printf("%d %d %d",
                (int)sizeof(*pack),
                (int)sizeof(exts),
                (int)(sizeof(exts) / sizeof((exts)[0])));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 96 6");
}

TEST(Compiler, sizeofStructMemberBoundFromGitArraySize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *calloc(unsigned long, unsigned long);
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_AN_ARRAY(arr) \
    BUILD_ASSERT_OR_ZERO(!__builtin_types_compatible_p(__typeof__(arr), \
        __typeof__(&(arr)[0])))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]) + BARF_UNLESS_AN_ARRAY(x))
        static struct {
            const char *name;
            unsigned optional:1;
        } exts[] = {
            {".pack"},
            {".rev", 1},
            {".mtimes", 1},
            {".bitmap", 1},
            {".promisor", 1},
            {".idx"},
        };
        struct tempfile;
        struct generated_pack {
            struct tempfile *tempfiles[ARRAY_SIZE(exts)];
        };
        int main(void) {
            struct generated_pack *pack;
            int i;
            pack = (struct generated_pack *)calloc(1, sizeof(*pack));
            for (i = 0; i < (int)ARRAY_SIZE(exts); i++) {
                pack->tempfiles[i] = (struct tempfile *)(long)(i + 1);
            }
            printf("%d %d %d %d",
                (int)sizeof(*pack),
                (int)ARRAY_SIZE(exts),
                (int)(long)pack->tempfiles[0],
                (int)(long)pack->tempfiles[5]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 6 1 6");
}

// git: repack.h forward-declares struct generated_pack and prototypes
// generated_pack_populate before exts[] and the struct body in repack.c.
// Parse completes the shared body, so SA of the prototype must not visit
// ARRAY_SIZE(exts) before exts is bound.
TEST(Compiler, sizeofStructMemberBoundAfterForwardPrototype) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct generated_pack;
        struct generated_pack *generated_pack_populate(void);
        static struct {
            const char *name;
            unsigned optional:1;
        } exts[] = {
            {".pack"},
            {".rev", 1},
            {".mtimes", 1},
            {".bitmap", 1},
            {".promisor", 1},
            {".idx"},
        };
#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)
#define BARF_UNLESS_AN_ARRAY(arr) \
    BUILD_ASSERT_OR_ZERO(!__builtin_types_compatible_p(__typeof__(arr), \
        __typeof__(&(arr)[0])))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]) + BARF_UNLESS_AN_ARRAY(x))
        struct tempfile;
        struct generated_pack {
            struct tempfile *tempfiles[ARRAY_SIZE(exts)];
        };
        struct generated_pack *generated_pack_populate(void) {
            return 0;
        }
        int main(void) {
            printf("%d %d", (int)sizeof(struct generated_pack), (int)ARRAY_SIZE(exts));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 6");
}

TEST(Compiler, sizeofUnionMemberBoundFromCompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[4];
        union U {
            void *slots[sizeof(exts) / sizeof((exts)[0])];
            unsigned long first;
        };
        int main(void) {
            printf("%d", (int)sizeof(union U));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("32");
}

TEST(Compiler, sizeofNestedStructMemberBoundFromCompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[6];
        struct Inner {
            void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
        };
        struct Outer {
            struct Inner inner;
            int tag;
        };
        int main(void) {
            struct Outer o;
            o.inner.tempfiles[5] = (void *)(long)5;
            o.tag = 7;
            printf("%d %d %d %d",
                (int)sizeof(struct Inner),
                (int)sizeof(struct Outer),
                (int)(long)o.inner.tempfiles[5],
                o.tag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 56 5 7");
}

TEST(Compiler, sizeofInlineNestedStructMemberBoundFromCompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[6];
        struct Outer {
            struct Inner {
                void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
            } inner;
            int tag;
        };
        int main(void) {
            struct Outer o;
            o.inner.tempfiles[5] = (void *)(long)5;
            o.tag = 7;
            printf("%d %d %d",
                (int)sizeof(struct Outer),
                (int)(long)o.inner.tempfiles[5],
                o.tag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("56 5 7");
}

TEST(Compiler, sizeofUntaggedAnonymousMemberBoundFromCompleteArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[6];
        struct Outer {
            struct {
                void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
            };
            int tag;
        };
        int main(void) {
            struct Outer o;
            o.tempfiles[5] = (void *)(long)5;
            o.tag = 7;
            printf("%d %d %d",
                (int)sizeof(struct Outer),
                (int)(long)o.tempfiles[5],
                o.tag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("56 5 7");
}

TEST(Compiler, sizeofStructMemberBoundWithCombinedDeclarator) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        static int exts[6];
        struct generated_pack {
            void *tempfiles[sizeof(exts) / sizeof((exts)[0])];
        } pack;
        int main(void) {
            pack.tempfiles[5] = (void *)(long)6;
            printf("%d %d", (int)sizeof(pack), (int)(long)pack.tempfiles[5]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 6");
}

TEST(Compiler, structMemberRuntimeArrayBoundIsError) {
    SourceProgram program{R"prg(
        int main(void) {
            int n;
            n = 3;
            struct S {
                int a[n];
            };
            struct S s;
            s.a[0] = 1;
            return s.a[0];
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("array size is not a non-negative constant expression");
}

} // namespace
