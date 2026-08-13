#include "TestFixtures.h"

namespace {

// git ARRAY_SIZE: sizeof/sizeof + BUILD_ASSERT_OR_ZERO(!types_compatible_p(arr, &arr[0])).
// Must report the true element count (not N-1).
TEST(Preprocessor, arraySizeWithBuildAssertOrZero) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    program.compile();
    program.runAndExpect("3");
}

// git MOVE_ARRAY / BARF_UNLESS_COPYABLE:
//   sizeof(*(dst)) + (sizeof(char [1 - 2*!(types_compatible_p(...))]) - 1)
// When types_compatible_p is 0 (product approx), the char[] bound is -1.
// Must still contribute 0 so element size stays sizeof(pointer)=8, not 7.
// A wrong size memmoves 7 bytes per pointer and corrupts argv/pathspec (git add
// with many paths: "pathspec '<.text garbage>' did not match").
TEST(Preprocessor, buildAssertOrZeroFalseContributesZero) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            int false_assert = (int)(sizeof(char [1 - 2*!(0)]) - 1);
            int true_assert = (int)(sizeof(char [1 - 2*!(1)]) - 1);
            printf("%d %d", false_assert, true_assert);
            return 0;
        }
    )prg", "build_assert_or_zero"};
    program.compile();
    program.runAndExpect("0 0");
}

TEST(Preprocessor, moveArrayElementSizeIsPointerWidth) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    program.compile();
    program.runAndExpect("8");
}

// git.c: MOVE_ARRAY(new_argv - option_count, new_argv, count)
TEST(Preprocessor, moveArrayPointerDiffTypesCompatibleP) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char *store[4];
            char **new_argv = store + 1;
            int option_count = 1;
            int sz = (int)(sizeof(*(new_argv - option_count))
                + (sizeof(char [1 - 2*!(__builtin_types_compatible_p(
                        __typeof__(*(new_argv - option_count)),
                        __typeof__(*new_argv)))]) - 1));
            printf("%d", sz);
            return 0;
        }
    )prg", "move_array_ptr_diff"};
    program.compile();
    program.runAndExpect("8");
}

// DUP_ARRAY: COPY_ARRAY(ALLOC_ARRAY(dst), src) -> typeof(*(dst = malloc(...)))
TEST(Preprocessor, dupArrayAssignmentTypesCompatibleP) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            char **dst;
            char **src;
            int sz = (int)(sizeof(*((dst) = src))
                + (sizeof(char [1 - 2*!(__builtin_types_compatible_p(
                        __typeof__(*((dst) = src)),
                        __typeof__(*src)))]) - 1));
            printf("%d", sz);
            return 0;
        }
    )prg", "dup_array_assign"};
    program.compile();
    program.runAndExpect("8");
}

// In-place rearrange of many path pointers (parse_options / MOVE_ARRAY shape).
TEST(Preprocessor, moveArrayPreservesManyPointers) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    program.compile();
    program.runAndExpect("bcdefghijkl");
}

// __builtin_offsetof expands to a pointer cast + member address expression.
// System V natural layout: int a at 0, int b at 4.
TEST(Preprocessor, expandsBuiltinOffsetof) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct S {
            int a;
            int b;
        };
        int main() {
            printf("%d", (int)__builtin_offsetof(struct S, b));
            return 0;
        }
    )prg", "sanitize_offsetof"};
    program.compile();
    program.runAndExpect("4");
}

// Git's OFFSETOF_VAR(ptr, member) expands (under gcc -E with __GNUC__) to
// __builtin_offsetof(__typeof__(*ptr), member). Member need not be first:
// struct test_entry { int padding; struct hashmap_entry ent; ... } has ent at 8.
// Rewriting typeof-based offsetof to 0 breaks container_of / hashmap_put_entry.
TEST(Preprocessor, builtinOffsetofTypeofStarNonFirstMember) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    program.compile();
    program.runAndExpect("8 1 K");
}

// OFFSETOF_VAR(ptr, member) expands to &ptr->member - ptr. The loop macro
// hashmap_for_each_entry starts with var = NULL, so the bound must not load
// through the null pointer when forming &NULL->member.
TEST(Preprocessor, addressOfArrowMemberDoesNotLoad) {
    SourceProgram program{R"prg(#include <stdio.h>
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
    SourceProgram program{R"prg(#include <stdio.h>
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
    program.compile();
    program.runAndExpect("value1");
}

} // namespace
