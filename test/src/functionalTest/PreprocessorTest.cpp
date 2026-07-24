#include "TestFixtures.h"

#include <fstream>
#include <cstdlib>

#include "ResourceHelpers.h"

namespace {

TEST(Preprocessor, preprocessesIncludeAndDefine) {
    // Write a header next to the generated source and compile a program that includes it.
    std::string dir = getTestResourcePath("programs/tmp");
    std::string headerPath = dir + "/preproc_header.h";
    {
        std::ofstream header(headerPath);
        header << "#define ANSWER 42\n";
        header << "int helper(void);\n";
    }

    SourceProgram program{R"prg(
        #include "preproc_header.h"
        int helper(void) {
            return ANSWER;
        }
        int main() {
            printf("%d", helper());
            return 0;
        }
    )prg", "preproc_include"};
    program.compileWithPreprocess();
    program.runAndExpect("42");
}

TEST(Preprocessor, preprocessesConditional) {
    SourceProgram program{R"prg(
        #define FEATURE 1
        int main() {
        #if FEATURE
            printf("%d", 1);
        #else
            printf("%d", 0);
        #endif
            return 0;
        }
    )prg", "preproc_cond"};
    program.compileWithPreprocess();
    program.runAndExpect("1");
}

// Git's PRI* macros expand to adjacent fragments like "%" "l" "u". The
// Adjacent string tokens must merge even when an earlier case '"': appears in the TU.
TEST(Preprocessor, concatenatesAdjacentStringsAfterCharLiteral) {
    SourceProgram program{R"prg(
        int classify(char c) {
            switch (c) {
            case '"':
                return 1;
            default:
                return 0;
            }
        }
        int main() {
            unsigned long n = 42;
            printf("%""l" "u%d", n, classify('"'));
            return 0;
        }
    )prg", "strconcat_after_char"};
    program.compileWithPreprocess();
    program.runAndExpect("421");
}

// Scanner/frontend must strip __attribute__((...)) so post-gcc -E output is parseable.
TEST(Preprocessor, stripsAttribute) {
    SourceProgram program{R"prg(
        int helper(void) __attribute__((unused));
        int helper(void) {
            return 7;
        }
        int main() {
            printf("%d", helper());
            return 0;
        }
    )prg", "sanitize_attr"};
    program.compileWithPreprocess();
    program.runAndExpect("7");
}

// __extension__ markers appear in glibc headers; strip them.
TEST(Preprocessor, stripsExtension) {
    SourceProgram program{R"prg(
        int main() {
            __extension__ int x;
            x = 3;
            printf("%d", x);
            return 0;
        }
    )prg", "sanitize_ext"};
    program.compileWithPreprocess();
    program.runAndExpect("3");
}

// inline is stripped as a specifier word without corrupting identifiers like "pipeline".
TEST(Preprocessor, stripsInlineKeepsPipelineIdent) {
    SourceProgram program{R"prg(
        inline int add1(int x) { return x + 1; }
        int main() {
            int pipeline;
            pipeline = add1(8);
            printf("%d", pipeline);
            return 0;
        }
    )prg", "sanitize_inline"};
    program.compileWithPreprocess();
    program.runAndExpect("9");
}

// The word "inline" inside string literals must survive inline-specifier stripping
// (git fast-import: skip_prefix(p, "inline ", &p) for "M 644 inline path").
TEST(Preprocessor, preservesInlineInsideStringLiteral) {
    SourceProgram program{R"prg(
        int main() {
            printf("[%s]", "inline");
            printf("[%s]", "644 inline file");
            return 0;
        }
    )prg", "sanitize_inline_str"};
    program.compileWithPreprocess();
    program.runAndExpect("[inline][644 inline file]");
}

// __builtin_constant_p folds to 0 so both branches remain parseable constants-free.
TEST(Preprocessor, foldsBuiltinConstantP) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = 5;
            if (__builtin_constant_p(x)) {
                printf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg", "sanitize_bcp"};
    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// __builtin_types_compatible_p is rewritten to 0 (see ARRAY_SIZE / BARF_UNLESS_AN_ARRAY).
TEST(Preprocessor, foldsBuiltinTypesCompatibleP) {
    SourceProgram program{R"prg(
        int main() {
            if (__builtin_types_compatible_p(int, int)) {
                printf("%d", 1);
            } else {
                printf("%d", 0);
            }
            return 0;
        }
    )prg", "sanitize_btc"};
    program.compileWithPreprocess();
    program.runAndExpect("0");
}

// git ARRAY_SIZE: sizeof/sizeof + BUILD_ASSERT_OR_ZERO(!types_compatible_p(arr, &arr[0])).
// Must report the true element count (not N-1).
TEST(Preprocessor, arraySizeWithBuildAssertOrZero) {
    SourceProgram program{R"prg(
        struct Entry {
            const char *a;
            const char *b;
        };
        static struct Entry settings[] = {
            { "e0", "c0" },
            { "e1", "c1" },
            { "e2", "c2" },
        };
        int main() {
            int n = (int)(sizeof(settings) / sizeof((settings)[0])
                + (sizeof(char [1 - 2*!(!__builtin_types_compatible_p(__typeof__(settings), __typeof__(&(settings)[0])))]) - 1));
            printf("%d", n);
            return 0;
        }
    )prg", "array_size_barf"};
    program.compileWithPreprocess();
    program.runAndExpect("3");
}

// git MOVE_ARRAY / BARF_UNLESS_COPYABLE:
//   sizeof(*(dst)) + (sizeof(char [1 - 2*!(types_compatible_p(...))]) - 1)
// When types_compatible_p is 0 (product approx), the char[] bound is -1.
// Must still contribute 0 so element size stays sizeof(pointer)=8, not 7.
// A wrong size memmoves 7 bytes per pointer and corrupts argv/pathspec (git add
// with many paths: "pathspec '<.text garbage>' did not match").
TEST(Preprocessor, buildAssertOrZeroFalseContributesZero) {
    SourceProgram program{R"prg(
        int main() {
            int false_assert = (int)(sizeof(char [1 - 2*!(0)]) - 1);
            int true_assert = (int)(sizeof(char [1 - 2*!(1)]) - 1);
            printf("%d %d", false_assert, true_assert);
            return 0;
        }
    )prg", "build_assert_or_zero"};
    program.compileWithPreprocess();
    program.runAndExpect("0 0");
}

TEST(Preprocessor, moveArrayElementSizeIsPointerWidth) {
    SourceProgram program{R"prg(
        int main() {
            const char **dst;
            const char **src;
            int sz = (int)(sizeof(*(dst))
                + (sizeof(char [1 - 2*!(__builtin_types_compatible_p(
                        __typeof__(*(dst)), __typeof__(*(src))))]) - 1));
            printf("%d", sz);
            return 0;
        }
    )prg", "move_array_elem_size"};
    program.compileWithPreprocess();
    program.runAndExpect("8");
}

// In-place rearrange of many path pointers (parse_options / MOVE_ARRAY shape).
TEST(Preprocessor, moveArrayPreservesManyPointers) {
    SourceProgram program{R"prg(
        void move_array(void *dst, const void *src, unsigned long n, unsigned long size) {
            unsigned char *d = (unsigned char *)dst;
            const unsigned char *s = (const unsigned char *)src;
            unsigned long i;
            if (n) {
                for (i = 0; i < n * size; i = i + 1) {
                    d[i] = s[i];
                }
            }
        }
        int main() {
            const char *paths[16];
            const char *a = "a"; const char *b = "b"; const char *c = "c";
            const char *d = "d"; const char *e = "e"; const char *f = "f";
            const char *g = "g"; const char *h = "h"; const char *i = "i";
            const char *j = "j"; const char *k = "k"; const char *l = "l";
            paths[0] = a; paths[1] = b; paths[2] = c; paths[3] = d;
            paths[4] = e; paths[5] = f; paths[6] = g; paths[7] = h;
            paths[8] = i; paths[9] = j; paths[10] = k; paths[11] = l;
            /* Shift paths[1..] down like parse_options_end MOVE_ARRAY. */
            {
                const char **dst = paths;
                const char **src = paths + 1;
                unsigned long n = 11;
                unsigned long size = sizeof(*(dst))
                    + (sizeof(char [1 - 2*!(__builtin_types_compatible_p(
                            __typeof__(*(dst)), __typeof__(*(src))))]) - 1);
                move_array(dst, src, n, size);
            }
            printf("%s%s%s%s%s%s%s%s%s%s%s",
                paths[0], paths[1], paths[2], paths[3], paths[4],
                paths[5], paths[6], paths[7], paths[8], paths[9], paths[10]);
            return 0;
        }
    )prg", "move_array_many_ptrs"};
    program.compileWithPreprocess();
    program.runAndExpect("bcdefghijkl");
}

// linux/types.h shape after gcc -E (no string rewrite layer):
// typedef __signed__ __int128 __s128 __attribute__((aligned(16)));
// Must parse and register typedef names for later use.
TEST(Preprocessor, linuxInt128TypedefsParseAndUse) {
    SourceProgram program{R"prg(
        typedef __signed__ __int128 __s128 __attribute__((aligned(16)));
        typedef unsigned __int128 __u128 __attribute__((aligned(16)));
        int main() {
            __s128 a;
            __u128 b;
            a = 1;
            b = 2;
            printf("%d %d %d %d", (int)a, (int)b, (int)sizeof(__s128), (int)sizeof(__u128));
            return 0;
        }
    )prg", "linux_int128_typedefs"};
    program.compileWithPreprocess();
    // Approximate as long long: 8-byte size (product stand-in, not true 16).
    program.runAndExpect("1 2 8 8");
}

// glibc _FloatN declarations after gcc -E must type-check as float/double stand-ins.
TEST(Preprocessor, extendedFloatTypesAsStandIns) {
    SourceProgram program{R"prg(
        int main() {
            _Float32 a;
            _Float64 b;
            _Float128 c;
            a = 1.0f;
            b = 2.0;
            c = 3.0;
            printf("%d %d %d", (int)a, (int)b, (int)c);
            return 0;
        }
    )prg", "floatn_standins"};
    program.compileWithPreprocess();
    program.runAndExpect("1 2 3");
}

// _Bool is 1 byte (SysV AMD64 / GCC ABI), not int.
TEST(Preprocessor, mapsBoolToOneByte) {
    SourceProgram program{R"prg(
        int main() {
            _Bool flag;
            flag = 1;
            printf("%d %d", flag, (int)sizeof(flag));
            return 0;
        }
    )prg", "sanitize_bool"};
    program.compileWithPreprocess();
    program.runAndExpect("1 1");
}

// bool arrays pack 1 byte per element (git xdiff changed[] / memcpy by line count).
TEST(Preprocessor, boolArrayIsOneBytePerElement) {
    SourceProgram program{R"prg(
        int main() {
            _Bool arr[4];
            arr[0] = 1;
            arr[1] = 0;
            arr[2] = 1;
            arr[3] = 0;
            printf("%d %d %d %d %d", (int)sizeof(arr), arr[0], arr[1], arr[2], arr[3]);
            return 0;
        }
    )prg", "bool_array_pack"};
    program.compileWithPreprocess();
    program.runAndExpect("4 1 0 1 0");
}

// expat: typedef unsigned char XML_Bool;  Naive replaceAll("_Bool", ...) turns
// this into "XMLunsigned char" and breaks http-push.c (git http-push).
TEST(Preprocessor, doesNotCorruptXmlBoolTypedef) {
    SourceProgram program{R"prg(
        typedef unsigned char XML_Bool;
        XML_Bool XML_ParserReset(void *parser, const char *encoding);
        XML_Bool XML_ParserReset(void *parser, const char *encoding) {
            (void)parser;
            (void)encoding;
            return 1;
        }
        int main() {
            XML_Bool ok;
            ok = XML_ParserReset(0, 0);
            printf("%d %d", ok, (int)sizeof(XML_Bool));
            return 0;
        }
    )prg", "sanitize_xml_bool"};
    program.compileWithPreprocess();
    program.runAndExpect("1 1");
}

// __builtin_offsetof expands to a pointer cast + member address expression.
// System V natural layout: int a at 0, int b at 4.
TEST(Preprocessor, expandsBuiltinOffsetof) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        int main() {
            printf("%d", (int)__builtin_offsetof(struct S, b));
            return 0;
        }
    )prg", "sanitize_offsetof"};
    program.compileWithPreprocess();
    program.runAndExpect("4");
}

// Git's OFFSETOF_VAR(ptr, member) expands (under gcc -E with __GNUC__) to
// __builtin_offsetof(__typeof__(*ptr), member). Member need not be first:
// struct test_entry { int padding; struct hashmap_entry ent; ... } has ent at 8.
// Rewriting typeof-based offsetof to 0 breaks container_of / hashmap_put_entry.
TEST(Preprocessor, builtinOffsetofTypeofStarNonFirstMember) {
    SourceProgram program{R"prg(
        struct ent {
            struct ent *next;
            unsigned int hash;
        };
        struct test_entry {
            int padding;
            struct ent ent;
            char key[1];
        };
        static void *container_of_or_null_offset(void *ptr, unsigned long offset) {
            return ptr ? (char *)ptr - offset : 0;
        }
        int main() {
            struct test_entry e;
            struct test_entry *entry = &e;
            e.padding = 0x11;
            e.ent.next = 0;
            e.ent.hash = 0x22;
            e.key[0] = 'K';
            /* same expansion as git OFFSETOF_VAR(entry, ent) after gcc -E */
            unsigned long off = __builtin_offsetof(__typeof__(*entry), ent);
            void *back = container_of_or_null_offset(&entry->ent, off);
            struct test_entry *got = (struct test_entry *)back;
            printf("%d %d %c", (int)off, got == entry, got->key[0]);
            return 0;
        }
    )prg", "offsetof_typeof_star"};
    program.compileWithPreprocess();
    program.runAndExpect("8 1 K");
}

// OFFSETOF_VAR(ptr, member) expands to &ptr->member - ptr. The loop macro
// hashmap_for_each_entry starts with var = NULL, so the bound must not load
// through the null pointer when forming &NULL->member.
TEST(Preprocessor, addressOfArrowMemberDoesNotLoad) {
    SourceProgram program{R"prg(
        struct ent {
            struct ent *next;
            unsigned int hash;
        };
        struct test_entry {
            int padding;
            struct ent ent;
        };
        static unsigned long offsetof_var(struct test_entry *entry) {
            return (unsigned long)&entry->ent - (unsigned long)entry;
        }
        int main() {
            struct test_entry *p;
            p = 0;
            printf("%d", (int)offsetof_var(p));
            return 0;
        }
    )prg", "addr_of_arrow_null"};
    program.compile();
    program.runAndExpect("8");
}

// Full git-style replace: put returns old entry recovered via container_of with
// OFFSETOF_VAR; flex key/value must remain readable (not empty from wrong base).
TEST(Preprocessor, containerOfTypeofOffsetPreservesFlexPayload) {
    SourceProgram program{R"prg(
        #include <string.h>
        #include <stdlib.h>
        struct hashmap_entry {
            struct hashmap_entry *next;
            unsigned int hash;
        };
        struct test_entry {
            int padding;
            struct hashmap_entry ent;
            char key[];
        };
        static void *container_of_or_null_offset(void *ptr, unsigned long offset) {
            return ptr ? (char *)ptr - offset : 0;
        }
        static const char *get_value(const struct test_entry *e) {
            return e->key + strlen(e->key) + 1;
        }
        int main() {
            size_t klen = 4, vlen = 6;
            struct test_entry *entry = malloc(sizeof(*entry) + klen + vlen + 2);
            entry->padding = 0;
            entry->ent.next = 0;
            entry->ent.hash = 1;
            memcpy(entry->key, "key1", 5);
            memcpy(entry->key + 5, "value1", 7);
            /* simulate hashmap_put returning &old->ent, then container_of */
            struct hashmap_entry *he = &entry->ent;
            struct test_entry *old = container_of_or_null_offset(
                he, __builtin_offsetof(__typeof__(*entry), ent));
            printf("%s", get_value(old));
            free(entry);
            return 0;
        }
    )prg", "container_of_flex"};
    program.compileWithPreprocess();
    program.runAndExpect("value1");
}

// Remaining #pragma lines after -E -P must be dropped.
TEST(Preprocessor, dropsPragmaLines) {
    SourceProgram program{R"prg(
        #pragma GCC diagnostic ignored "-Wunused"
        int main() {
            printf("%d", 1);
            return 0;
        }
    )prg", "sanitize_pragma"};
    program.compileWithPreprocess();
    program.runAndExpect("1");
}

// -U cancels a prior -D for the preprocessor (order in Compiler::preprocess is -D then -U).
TEST(Preprocessor, preprocessesWithUndefine) {
    SourceProgram program{R"prg(
        int main() {
        #ifdef FEATURE
            printf("%d", 1);
        #else
            printf("%d", 0);
        #endif
            return 0;
        }
    )prg", "preproc_undef"};

    program.compileWithArgs({
            "-DFEATURE=1",
            "-UFEATURE",
            program.getSourceFilePath()
    });
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("0");
}

// git compat/obstack.h gates GNU statement expressions on
//   defined __GNUC__ && defined __STDC__ && __STDC__
// Forcing __STDC__=0 during preprocess selects the portable comma-expression
// macros (kwset). This header mimics that gate: with stmt-exprs the frontend
// cannot parse; with the non-STDC path the simple form must compile and run.
TEST(Preprocessor, forcesNonGnucObstackStyleViaStdc0) {
    std::string dir = getTestResourcePath("programs/tmp");
    std::string headerPath = dir + "/obstack_style.h";
    {
        std::ofstream header(headerPath);
        header <<
            "#if defined __GNUC__ && defined __STDC__ && __STDC__\n"
            "# define GET_PLUS_ONE(x) ({ int __t = (x); __t + 1; })\n"
            "#else\n"
            "# define GET_PLUS_ONE(x) ((x) + 1)\n"
            "#endif\n";
    }

    SourceProgram program{R"prg(
        #include "obstack_style.h"
        int main() {
            printf("%d", GET_PLUS_ONE(41));
            return 0;
        }
    )prg", "preproc_stdc0_obstack"};
    program.compileWithPreprocess();
    program.runAndExpect("42");
}

// Non-GNUC obstack_alloc expands to nested comma/ternary expressions with a
// temp union member. Smoke-test that pattern after preprocess.
TEST(Preprocessor, nonGnucObstackCommaExpression) {
    SourceProgram program{R"prg(
        struct H {
            int temp;
            char *base;
            char *next;
        };
        int main() {
            struct H h;
            char buf[16];
            int n;
            h.base = buf;
            h.next = buf;
            n = 4;
            /* mimic: (h.temp = n, h.next += h.temp, h.base) */
            {
                char *p;
                p = (h.temp = n, h.next = h.next + h.temp, h.base);
                p[0] = 'a';
                p[1] = 'b';
                printf("%c%c %d", h.base[0], h.base[1], (int)(h.next - h.base));
            }
            return 0;
        }
    )prg", "obstack_comma"};
    program.compileWithPreprocess();
    program.runAndExpect("ab 4");
}

// git t/unit-tests/clar/clar.c uses L"NULL" for wide-string assert messages.
// The subset frontend has no wide-string tokens; strip the L prefix so L"..."
// becomes a regular string literal (type checks are loose enough for clar).
TEST(Preprocessor, wideStringLiteralPrefixStripped) {
    SourceProgram program{R"prg(
        int main() {
            const char *p;
            p = L"NULL";
            printf("%s", p);
            return 0;
        }
    )prg", "wide_str_prefix"};
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("NULL");
}

// GNU statement expressions are not supported. Capture the parse failure so
// we do not silently regress if a workaround stops covering real code.
TEST(Preprocessor, statementExpressionIsRejected) {
    SourceProgram program{R"prg(
        int main() {
            int x;
            x = ({ int y; y = 1; y + 2; });
            printf("%d", x);
            return 0;
        }
    )prg", "stmt_expr_reject"};
    // Feed already-preprocessed form: the failure is in the parser, not cpp.
    program.compileWithArgs({
            "--no-preprocess",
            program.getSourceFilePath()
    });
    ASSERT_FALSE(program.isCompiled());
    program.assertCompilationErrors("unexpected token");
}

// libcurl's typecheck-gcc.h wraps curl_easy_setopt in ({ ... }) under
// !CURL_DISABLE_TYPECHECK (git imap-send.c with USE_CURL_FOR_IMAP_SEND).
// Preprocess must inject CURL_DISABLE_TYPECHECK so the plain prototype form
// is selected - same idea as -D__STDC__=0 for obstack.
TEST(Preprocessor, disablesCurlTypecheckByDefault) {
    std::string dir = getTestResourcePath("programs/tmp");
    std::string headerPath = dir + "/curl_typecheck_style.h";
    {
        std::ofstream header(headerPath);
        header <<
            "int setopt_impl(void *h, int opt, const char *v);\n"
            "#if !defined(CURL_DISABLE_TYPECHECK)\n"
            "# define easy_setopt(h, o, v) "
            "({ if (0) { } setopt_impl((h), (o), (v)); })\n"
            "#else\n"
            "# define easy_setopt(h, o, v) setopt_impl((h), (o), (v))\n"
            "#endif\n";
    }

    SourceProgram program{R"prg(
        #include "curl_typecheck_style.h"
        int setopt_impl(void *h, int opt, const char *v) {
            (void)h;
            (void)opt;
            printf("%s", v);
            return 0;
        }
        int main() {
            void *c;
            c = 0;
            easy_setopt(c, 1, "ok");
            return 0;
        }
    )prg", "preproc_curl_typecheck"};
    // No manual -DCURL_DISABLE_TYPECHECK: the driver must supply it.
    program.compileWithPreprocess();
    ASSERT_TRUE(program.isCompiled());
    program.runAndExpect("ok");
}

// git pack-bitmap expands __builtin_ctzll in the frontend; must not rely on
// unsupported statement-expression syntax in the expansion.
TEST(Preprocessor, builtinCtzll) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long long x;
            x = 8;
            printf("%d", __builtin_ctzll(x));
            x = 1;
            printf(" %d", __builtin_ctzll(x));
            x = 16;
            printf(" %d", __builtin_ctzll(x));
            return 0;
        }
    )prg"};
    program.compileWithPreprocess();
    program.runAndExpect("3 0 4");
}

// git tree-diff uses xalloca -> alloca -> __builtin_alloca for small parent
// arrays. Rewrite to malloc so the call is a normal library function.
TEST(Preprocessor, builtinAlloca) {
    SourceProgram program{R"prg(
        void *malloc(unsigned long);
        int main() {
            int n;
            int *p;
            n = 3;
            p = (int *)__builtin_alloca((unsigned long)n * 8);
            p[0] = 1;
            p[1] = 2;
            p[2] = 3;
            printf("%d %d %d", p[0], p[1], p[2]);
            return 0;
        }
    )prg", "builtin_alloca"};
    program.compileWithPreprocess();
    program.runAndExpect("1 2 3");
}

// git pack-revindex uses ntohl(*off_32++) -> __builtin_bswap32(*off_32++).
// The rewrite must evaluate the argument once; expanding to four mask/shift
// terms that each mention the arg re-runs the post-increment (stride 16 instead
// of 4) and mixes bytes from four consecutive words (wrong host value).
TEST(Preprocessor, builtinBswap32ArgEvaluatedOnce) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int words[4];
            unsigned int *p;
            unsigned int v;
            /* big-endian 13180, 13794, 13923, 13882 as host loads on LE */
            words[0] = 0x7c330000u;
            words[1] = 0xe2350000u;
            words[2] = 0x63360000u;
            words[3] = 0x3a360000u;
            p = words;
            v = __builtin_bswap32(*p++);
            printf("%u %ld", v, (long)(p - words));
            v = __builtin_bswap32(*p++);
            printf(" %u %ld", v, (long)(p - words));
            return 0;
        }
    )prg", "builtin_bswap32_once"};
    program.compileWithPreprocess();
    /* 13180 and 13794 with pointer advanced by 1 then 2 entries */
    program.runAndExpect("13180 1 13794 2");
}

TEST(Preprocessor, builtinBswap16ArgEvaluatedOnce) {
    SourceProgram program{R"prg(
        int main() {
            unsigned short words[2];
            unsigned short *p;
            unsigned short v;
            words[0] = 0x3412;
            words[1] = 0x7856;
            p = words;
            v = __builtin_bswap16(*p++);
            printf("%u %ld", (unsigned)v, (long)(p - words));
            return 0;
        }
    )prg", "builtin_bswap16_once"};
    program.compileWithPreprocess();
    program.runAndExpect("4660 1");
}

TEST(Preprocessor, builtinBswap64ArgEvaluatedOnce) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long long words[2];
            unsigned long long *p;
            unsigned long long v;
            words[0] = 0x0102030405060708ull;
            words[1] = 0x1122334455667788ull;
            p = words;
            v = __builtin_bswap64(*p++);
            printf("%lu %ld", (unsigned long)v, (long)(p - words));
            return 0;
        }
    )prg", "builtin_bswap64_once"};
    program.compileWithPreprocess();
    /* 0x0807060504030201 */
    program.runAndExpect("578437695752307201 1");
}

// GNU keyword spellings from system headers must strip so the subset frontend
// can parse post-gcc -E output (__restrict / __inline__ / __const / ...).
TEST(Preprocessor, stripsGnuKeywordSpellings) {
    SourceProgram program{R"prg(
        __inline__ int add(int a, int b) {
            return a + b;
        }
        int take(__restrict int *p) {
            return *p;
        }
        __const int k = 3;
        int main() {
            int x = 4;
            printf("%d", add(take(&x), k));
            return 0;
        }
    )prg", "sanitize_gnu_kw"};
    program.compileWithPreprocess();
    program.runAndExpect("7");
}

// _Generic is rewritten to a single association (default when present).
TEST(Preprocessor, rewritesGenericDefault) {
    SourceProgram program{R"prg(
        int main() {
            int x = 0;
            int n = _Generic(x, int: 1, default: 2);
            printf("%d", n);
            return 0;
        }
    )prg", "sanitize_generic"};
    program.compileWithPreprocess();
    program.runAndExpect("2");
}

// UTF-8 string prefix u8"..." is dropped to a plain string token.
TEST(Preprocessor, dropsU8StringPrefix) {
    SourceProgram program{R"prg(
        int main() {
            const char *p;
            p = u8"hi";
            printf("%s", p);
            return 0;
        }
    )prg", "sanitize_u8"};
    program.compileWithPreprocess();
    program.runAndExpect("hi");
}

// __func__ / __FUNCTION__ / __PRETTY_FUNCTION__ → empty string:
// ProductContracts.funcIdentifiersMapToEmptyString (e2e) and
// ScannerFilter.mapsFuncIdentifiersToEmptyString (token stream).

// Assert-style macros emit __func__ / __PRETTY_FUNCTION__; map to empty string.
TEST(Preprocessor, mapsFuncIdentifiersToEmptyString) {
    SourceProgram program{R"prg(
        int main() {
            const char *a = __func__;
            const char *b = __FUNCTION__;
            const char *c = __PRETTY_FUNCTION__;
            printf("[%s][%s][%s]", a, b, c);
            return 0;
        }
    )prg", "sanitize_func"};
    program.compileWithPreprocess();
    program.runAndExpect("[][][]");
}

} // namespace
