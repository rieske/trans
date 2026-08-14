#include "TestFixtures.h"

namespace {

TEST(Compiler, fileScopeDecayedArrayPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        char *p = slop;
        int main(void) {
            slop[0] = 88;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 88");
}

TEST(Compiler, fileScopeGitStrbufInitShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        struct strbuf g = { .buf = slop };
        int main(void) {
            slop[0] = 65;
            printf("%d %d %d %d", g.alloc == 0, g.len == 0, g.buf == slop, g.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 65");
}

TEST(Compiler, fileScopeStrbufPositionalInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        struct strbuf g = { 0, 0, slop };
        int main(void) {
            printf("%d", g.buf == slop);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, automaticDecayedArrayPointerInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        int main(void) {
            char *p = slop;
            slop[0] = 88;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 88");
}

TEST(Compiler, automaticGitStrbufInitShape) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int main(void) {
            struct strbuf s = { .buf = slop };
            slop[0] = 65;
            printf("%d %d %d %d", s.alloc == 0, s.len == 0, s.buf == slop, s.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1 1 65");
}

TEST(Compiler, automaticStrbufPositionalInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int main(void) {
            struct strbuf s = { 0, 0, slop };
            slop[0] = 66;
            printf("%d %d", s.buf == slop, s.buf[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 66");
}

TEST(Compiler, functionScopeStaticStrbufInit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        char *buf_of(void) {
            static struct strbuf sb = { .buf = slop };
            return sb.buf;
        }
        int main(void) {
            slop[0] = 66;
            printf("%d %d", buf_of() == slop, buf_of()[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 66");
}

TEST(Compiler, functionScopeStaticStrbufPool) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        struct strbuf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int check(void) {
            static struct strbuf pool[2] = { { .buf = slop }, { .buf = slop } };
            return pool[0].buf == slop && pool[1].buf == slop;
        }
        int main(void) {
            printf("%d", check());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, functionScopeStaticDecayedLocalArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *buf_of(void) {
            static char slop[1];
            static char *p = slop;
            slop[0] = 67;
            return p;
        }
        int main(void) {
            char *p;
            p = buf_of();
            printf("%d %d", p[0], buf_of() == p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("67 1");
}

TEST(Compiler, fileScopeAddressOfObject) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 7;
        int *p = &g;
        int main(void) {
            printf("%d %d", p == &g, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7");
}

TEST(Compiler, functionScopeStaticAddressOfFileScope) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 9;
        int read(void) {
            static int *p = &g;
            return *p;
        }
        int main(void) {
            printf("%d", read());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9");
}

TEST(Compiler, decayedArrayEqualsAddressOfArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        char *a = slop;
        char (*b)[1] = &slop;
        int main(void) {
            printf("%d", a == (char *)b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeStringPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *p = "hi";
        int main(void) {
            printf("%s", p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hi");
}

TEST(Compiler, fileScopeFunctionPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int foo(void) {
            return 3;
        }
        int (*fp)(void) = foo;
        int main(void) {
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, fileScopeAddressOfFunction) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int foo(void) {
            return 4;
        }
        int (*fp)(void) = &foo;
        int main(void) {
            printf("%d", fp());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeNullPointerStillFolds) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char *p = 0;
        struct S {
            char *buf;
        };
        struct S g = { .buf = 0 };
        int main(void) {
            printf("%d %d", p == 0, g.buf == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, fileScopeExternArrayPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        extern char slop[1];
        char *p = slop;
        char slop[1];
        int main(void) {
            slop[0] = 70;
            printf("%d %d", p == slop, p[0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 70");
}

TEST(Compiler, staticInitRejectsAutomaticAddress) {
    SourceProgram program{R"prg(
        int main(void) {
            int x;
            static int *p = &x;
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, fileScopeFunctionInitToIntIsError) {
    SourceProgram program{R"prg(
        int foo(void) {
            return 1;
        }
        int g = foo;
        int main(void) {
            return g;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, fileScopeFunctionBraceInitToIntIsError) {
    SourceProgram program{R"prg(
        int foo(void) {
            return 1;
        }
        int g = { foo };
        int main(void) {
            return g;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("function designator used as a value is not supported");
}

TEST(Compiler, fileScopeArrayInitToFunctionPointerIsError) {
    SourceProgram program{R"prg(
        char slop[1];
        int (*fp)(void) = slop;
        int main(void) {
            return 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("type mismatch");
}

TEST(Compiler, fileScopeUnionAddressThenByteLastWins) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        char slop[1];
        union U {
            char *p;
            char c;
        };
        union U g = { .p = slop, .c = 1 };
        int main(void) {
            printf("%d", g.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeFloatFromInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        float f = 2;
        int main(void) {
            printf("%d", (int)(f + f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeDoubleFromInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        double d = 2;
        int main(void) {
            printf("%d", (int)(d + d));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("4");
}

TEST(Compiler, fileScopeIntFromFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = 2.5f;
        int main(void) {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2");
}

TEST(Compiler, fileScopeIntFromNegativeFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int g = -2.5f;
        int main(void) {
            printf("%d", g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

TEST(Compiler, fileScopeBoolFromFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 2.5f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeBoolFromSmallFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 0.1f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeBoolFromZeroFloat) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        bool b = 0.0f;
        int main(void) {
            printf("%d", b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0");
}

TEST(Compiler, fileScopeFloatFromDoubleLiteral) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        float f = 2.5;
        int main(void) {
            printf("%d", (int)(f + f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5");
}

TEST(Compiler, fileScopeStructFloatThenIntPack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            float f;
            int i;
        };
        struct S g = { 2.5f, 3 };
        int main(void) {
            printf("%d %d", (int)(g.f + g.f), g.i);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("5 3");
}

TEST(Compiler, fileScopeStructIntThenFloatPack) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int i;
            float f;
        };
        struct S g = { 3, 2.5f };
        int main(void) {
            printf("%d %d", g.i, (int)(g.f + g.f));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 5");
}

TEST(Compiler, fileScopeAddressOfMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S s = { 3, 9 };
        int *p = &s.b;
        int main(void) {
            printf("%d %d", p == &s.b, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 9");
}

TEST(Compiler, fileScopeAddressOfNestedMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Inner {
            int x;
            int y;
        };
        struct Outer {
            int pad;
            struct Inner in;
        };
        struct Outer o = { 0, { 4, 8 } };
        int *p = &o.in.y;
        int main(void) {
            printf("%d %d", p == &o.in.y, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8");
}

TEST(Compiler, fileScopeAddressOfArrayElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        int *p = &arr[1];
        int main(void) {
            printf("%d %d", p == &arr[1], *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, fileScopeArrayPlusInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        int *p = arr + 2;
        int main(void) {
            printf("%d %d", p == &arr[2], *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, fileScopeCastOfObjectAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S s = { 3, 9 };
        int *p = (int *)&s;
        int main(void) {
            printf("%d %d", p == (int *)&s, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, fileScopeIntegerCastToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *p = (void *)-1;
        char *q = (char *)1;
        int main(void) {
            printf("%d %d", p == (void *)-1, q == (char *)1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

TEST(Compiler, fileScopeAddressOfIndexedMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S arr[2] = { { 1, 2 }, { 3, 4 } };
        int *p = &arr[1].b;
        int main(void) {
            printf("%d %d", p == &arr[1].b, *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, fileScopeAddressOfMemberArrayElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int xs[3];
        };
        struct S s = { { 5, 6, 7 } };
        int *p = &s.xs[1];
        int main(void) {
            printf("%d %d", p == &s.xs[1], *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 6");
}

TEST(Compiler, fileScopeDecayedMemberArray) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int xs[3];
        };
        struct S s = { { 5, 6, 7 } };
        int *p = s.xs;
        int main(void) {
            printf("%d %d", p == &s.xs[0], *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 5");
}

TEST(Compiler, fileScopeIndirectionOfAddressConstant) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        int *p = &*(arr + 1);
        int main(void) {
            printf("%d %d", p == &arr[1], *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, fileScopeGitOptionMemberAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct opts {
            int flag;
            int extra;
        };
        struct option {
            void *value;
        };
        struct opts o;
        struct option table[1] = { { .value = &o.flag } };
        int main(void) {
            o.flag = 11;
            printf("%d %d", table[0].value == (void *)&o.flag, *(int *)table[0].value);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 11");
}

TEST(Compiler, functionScopeStaticAddressOfMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S s = { 3, 9 };
        int *get(void) {
            static int *p = &s.b;
            return p;
        }
        int main(void) {
            printf("%d %d", get() == &s.b, *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 9");
}

TEST(Compiler, functionScopeStaticAddressOfNestedMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Inner {
            int x;
            int y;
        };
        struct Outer {
            int pad;
            struct Inner in;
        };
        struct Outer o = { 0, { 4, 8 } };
        int *get(void) {
            static int *p = &o.in.y;
            return p;
        }
        int main(void) {
            printf("%d %d", get() == &o.in.y, *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8");
}

TEST(Compiler, functionScopeStaticAddressOfArrayElement) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        int *get(void) {
            static int *p = &arr[1];
            return p;
        }
        int main(void) {
            printf("%d %d", get() == &arr[1], *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, functionScopeStaticArrayPlusInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        int *get(void) {
            static int *p = arr + 2;
            return p;
        }
        int main(void) {
            printf("%d %d", get() == &arr[2], *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, functionScopeStaticCastOfObjectAddress) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S s = { 3, 9 };
        int *get(void) {
            static int *p = (int *)&s;
            return p;
        }
        int main(void) {
            printf("%d %d", get() == (int *)&s, *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 3");
}

TEST(Compiler, functionScopeStaticIntegerCastToPointer) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *get(void) {
            static void *p = (void *)-1;
            return p;
        }
        int main(void) {
            printf("%d", get() == (void *)-1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, functionScopeStaticAddressOfIndexedMember) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S arr[2] = { { 1, 2 }, { 3, 4 } };
        int *get(void) {
            static int *p = &arr[1].b;
            return p;
        }
        int main(void) {
            printf("%d %d", get() == &arr[1].b, *get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 4");
}

TEST(Compiler, staticInitRejectsAutomaticMemberAddress) {
    SourceProgram program{R"prg(
        int main(void) {
            struct S {
                int x;
            };
            struct S s;
            static int *p = &s.x;
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, staticInitRejectsNonConstantSubscript) {
    SourceProgram program{R"prg(
        int i = 1;
        int arr[3];
        int *p = &arr[i];
        int main(void) {
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, staticInitRejectsPointerObjectArrow) {
    SourceProgram program{R"prg(
        struct S {
            int x;
        };
        struct S s;
        struct S *ps = &s;
        int *p = &ps->x;
        int main(void) {
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, fileScopeStringCastToLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long g = (long)"hi";
        int main(void) {
            printf("%s", (char *)g);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("hi");
}

TEST(Compiler, fileScopeGitDefvalStringAsInteger) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef long intptr_t;
        struct option {
            intptr_t defval;
        };
        struct option o = { .defval = (intptr_t)"all" };
        int main(void) {
            printf("%s", (char *)o.defval);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("all");
}

TEST(Compiler, fileScopeGitDefvalEmptyString) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef long intptr_t;
        struct option {
            intptr_t defval;
        };
        struct option o = { .defval = (intptr_t)"" };
        int main(void) {
            printf("%d", ((char *)o.defval)[0] == 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, fileScopeAddressOfObjectCastToLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int x = 7;
        long p = (long)&x;
        int main(void) {
            printf("%d %d", p == (long)&x, *(int *)p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7");
}

TEST(Compiler, fileScopeAddressOfMemberCastToLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct S {
            int a;
            int b;
        };
        struct S s = { 3, 9 };
        long p = (long)&s.b;
        int main(void) {
            printf("%d %d", p == (long)&s.b, *(int *)p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 9");
}

TEST(Compiler, fileScopeArrayPlusCastToUnsignedLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int arr[3] = { 1, 2, 3 };
        unsigned long p = (unsigned long)(arr + 1);
        int main(void) {
            printf("%d %d", p == (unsigned long)&arr[1], *(int *)p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2");
}

TEST(Compiler, functionScopeStaticStringCastToLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        long get(void) {
            static long g = (long)"ok";
            return g;
        }
        int main(void) {
            printf("%s", (char *)get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

TEST(Compiler, functionScopeStaticGitDefvalString) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        typedef long intptr_t;
        struct option {
            intptr_t defval;
        };
        intptr_t get(void) {
            static struct option o = { .defval = (intptr_t)"all" };
            return o.defval;
        }
        int main(void) {
            printf("%s", (char *)get());
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("all");
}

TEST(Compiler, staticInitRejectsAutomaticAddressCastToLong) {
    SourceProgram program{R"prg(
        int main(void) {
            int x;
            static long p = (long)&x;
            return p == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

TEST(Compiler, staticInitRejectsNarrowIntFromAddress) {
    SourceProgram program{R"prg(
        int g = (int)"hi";
        int main(void) {
            return g == 0;
        }
    )prg"};
    program.compile();
    program.assertCompilationErrors("global initializer is not a constant expression");
}

} // namespace
