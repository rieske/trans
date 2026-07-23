#include "TestFixtures.h"

namespace {


// Positional brace initializer for local structs.
TEST(Compiler, structBraceInitializer) {
    SourceProgram program{R"prg(
        struct Point {
            int x;
            int y;
        };
        int main() {
            struct Point p = { 3, 4 };
            printf("%d %d", p.x, p.y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3 4");
}


// Nested brace init of an array member: struct S { unsigned char hash[8]; }
// s = { { 1 } } must set hash[0]=1 (rest 0). git u-example-decorate.c:
// struct object_id one_oid = { { 1 } }; without this, all OIDs are null and
// lookup_unknown_object returns the same object for every id.
TEST(Compiler, structNestedArrayBraceInitializer) {
    SourceProgram program{R"prg(
        struct object_id {
            unsigned char hash[8];
        };
        int main() {
            struct object_id one = { { 1 } };
            struct object_id two = { { 2 } };
            struct object_id multi = { { 1, 2, 3, 4 } };
            struct object_id desig = { .hash = { 5 } };
            struct object_id mixed = { { 6 }, };
            printf("%d %d %d %d %d %d %d %d %d %d",
                    (int)one.hash[0], (int)one.hash[1],
                    (int)two.hash[0], (int)two.hash[1],
                    (int)multi.hash[0], (int)multi.hash[1],
                    (int)multi.hash[2], (int)multi.hash[3],
                    (int)desig.hash[0], (int)mixed.hash[0]);
            return 0;
        }
    )prg", "nested_arr_brace"};
    program.compile();
    program.runAndExpect("1 0 2 0 1 2 3 4 5 6");
}


// C99 designated initializer (git strbuf: { .buf = slop }).
TEST(Compiler, structDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
            int c;
        };
        int main() {
            struct S s = { .b = 7 };
            printf("%d %d %d", s.a, s.b, s.c);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 7 0");
}


// Nested C99 designators (git REV_INFO_INIT: .pruning.flags.recursive = 1).
// Without path support, only the first segment is kept so .pruning becomes 1,
// which clobbers the first field of the nested struct (orderfile -> free(0x1)).
TEST(Compiler, structNestedDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct Flags {
            int recursive;
            int dense;
        };
        struct Prune {
            void *orderfile;
            int reverse;
            struct Flags flags;
        };
        struct Rev {
            int remerge_diff;
            struct Prune pruning;
            int limited;
        };
        int main() {
            struct Rev r = {
                .remerge_diff = 0,
                .pruning.flags.recursive = 1,
                .limited = 0
            };
            if (r.pruning.orderfile != 0) return 1;
            if (r.pruning.reverse != 0) return 2;
            if (r.pruning.flags.recursive != 1) return 3;
            if (r.pruning.flags.dense != 0) return 4;
            if (r.remerge_diff != 0 || r.limited != 0) return 5;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// Nested designators at file scope (static REV_INFO_INIT-style macros).
TEST(Compiler, globalStructNestedDesignatedInitializer) {
    SourceProgram program{R"prg(
        struct Flags {
            int recursive;
            int dense;
        };
        struct Prune {
            void *orderfile;
            int reverse;
            struct Flags flags;
        };
        struct Rev {
            int remerge_diff;
            struct Prune pruning;
            int limited;
        };
        struct Rev g = {
            .pruning.flags.recursive = 1
        };
        int main() {
            if (g.pruning.orderfile != 0) return 1;
            if (g.pruning.reverse != 0) return 2;
            if (g.pruning.flags.recursive != 1) return 3;
            if (g.pruning.flags.dense != 0) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// Multiple nested designators sharing a parent must not wipe prior stores.
TEST(Compiler, structNestedDesignatedMultipleLeaves) {
    SourceProgram program{R"prg(
        struct Flags {
            int recursive;
            int dense;
        };
        struct Outer {
            int x;
            struct Flags flags;
            int y;
        };
        int main() {
            struct Outer o = {
                .flags.recursive = 3,
                .flags.dense = 5,
                .y = 9
            };
            if (o.x != 0) return 1;
            if (o.flags.recursive != 3) return 2;
            if (o.flags.dense != 5) return 3;
            if (o.y != 9) return 4;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// git DIR_INIT / { 0 }: nested multi-word members and arrays inside a local
// struct must be fully zeroed, not only the first 8 bytes of each member.
// Incomplete zero-init left exclude_list_group[] as stack garbage so
// add_pattern_list memset'd a bad pl pointer.
TEST(Compiler, structBraceZeroInitNestedMultiWordAndArray) {
    SourceProgram program{R"prg(
        struct Inner {
            long a;
            long b;
            long c;
        };
        struct Group {
            int nr;
            int alloc;
            void *pl;
        };
        struct Dir {
            int flags;
            struct Inner nested;
            struct Group groups[3];
        };
        int main() {
            volatile char poison[64];
            int i;
            for (i = 0; i < 64; i = i + 1) {
                poison[i] = (char)0xAA;
            }
            struct Dir dir = { 0 };
            if (dir.flags != 0) return 1;
            if (dir.nested.a != 0 || dir.nested.b != 0 || dir.nested.c != 0) return 2;
            if (dir.groups[0].nr != 0 || dir.groups[0].alloc != 0 || dir.groups[0].pl != 0) return 3;
            if (dir.groups[1].nr != 0 || dir.groups[1].alloc != 0 || dir.groups[1].pl != 0) return 4;
            if (dir.groups[2].nr != 0 || dir.groups[2].alloc != 0 || dir.groups[2].pl != 0) return 5;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// Same as above but only { 0 } first member positional (DIR_INIT style).
TEST(Compiler, structBraceZeroInitPartialThenRestZero) {
    SourceProgram program{R"prg(
        struct Cell {
            long x;
            long y;
        };
        struct Row {
            int n;
            struct Cell cells[2];
        };
        int main() {
            volatile unsigned long junk;
            junk = 0xDEADBEEFCAFEBABEUL;
            struct Row row = { 0 };
            if (row.n != 0) return 1;
            if (row.cells[0].x != 0 || row.cells[0].y != 0) return 2;
            if (row.cells[1].x != 0 || row.cells[1].y != 0) return 3;
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// git oidset / khash: struct oidset s = { 0 } (or OID_ARRAY_INIT misused as
// { 0 }) where the sole member is a nested multi-word struct with pointer
// fields. Scalar 0 must zero the entire nested struct, not an 8-byte store
// at offset 0 (free(): invalid pointer in kh_release_oid_set / clone).
TEST(Compiler, structBraceZeroInitFirstMemberIsNestedStruct) {
    SourceProgram program{R"prg(
        struct Kh {
            unsigned n_buckets;
            unsigned size;
            unsigned n_occupied;
            unsigned upper_bound;
            int *flags;
            int *keys;
            int *vals;
        };
        struct Oidset {
            struct Kh set;
        };
        void poison_stack(void) {
            volatile char buf[256];
            int i;
            for (i = 0; i < 256; i = i + 1) {
                buf[i] = (char)0xA5;
            }
        }
        int main() {
            int j;
            int nonzero;
            unsigned char *p;
            poison_stack();
            {
                struct Oidset o = { 0 };
                if (o.set.n_buckets != 0 || o.set.size != 0) return 1;
                if (o.set.n_occupied != 0 || o.set.upper_bound != 0) return 2;
                if (o.set.flags != 0 || o.set.keys != 0 || o.set.vals != 0) return 3;
                p = (unsigned char *)&o;
                nonzero = 0;
                for (j = 0; j < (int)sizeof(o); j = j + 1) {
                    if (p[j] != 0) {
                        nonzero = nonzero + 1;
                    }
                }
                if (nonzero != 0) return 4;
            }
            printf("ok");
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}


// git reftable_stack::add: struct reftable_log_record logs[2] = { 0 };
// Scalar 0 must zero the whole first element (including nested union fields),
// not store one 8-byte zero over the first member and leave the rest as stack
// garbage (name/email pointers -> free(): invalid size).
TEST(Compiler, structArrayBraceZeroInitFullyZerosElements) {
    SourceProgram program{R"prg(
        struct Log {
            char *refname;
            unsigned long update_index;
            int value_type;
            unsigned char new_hash[32];
            unsigned char old_hash[32];
            char *name;
            char *email;
            char *message;
        };
        int main() {
            volatile char poison[64];
            int i;
            for (i = 0; i < 64; i = i + 1) {
                poison[i] = (char)0xA5;
            }
            struct Log logs[2] = { 0 };
            if (logs[0].refname != 0 || logs[0].name != 0 || logs[0].email != 0) return 1;
            if (logs[0].message != 0 || logs[0].value_type != 0) return 2;
            if (logs[0].update_index != 0 || logs[0].new_hash[0] != 0 || logs[0].old_hash[31] != 0) return 3;
            if (logs[1].refname != 0 || logs[1].name != 0 || logs[1].email != 0) return 4;
            if (logs[1].message != 0 || logs[1].value_type != 0) return 5;
            logs[0].email = (char *)1;
            logs[0].value_type = 1;
            if (logs[0].email == 0 || logs[0].name != 0) return 6;
            printf("ok");
            return 0;
        }
    )prg", "struct_arr_zero"};
    program.compile();
    program.runAndExpect("ok");
}


// git cmd_version: local incomplete array of structs with designated inits
// (parse-options OPT_BOOL / OPT_END tables).
TEST(Compiler, localIncompleteStructArrayDesignatedInit) {
    SourceProgram program{R"prg(
        struct Opt {
            int type;
            int short_name;
            const char *long_name;
            int *value;
            int defval;
        };
        int main() {
            int build_options;
            build_options = 0;
            struct Opt options[] = {
                { .type = 7, .short_name = 0, .long_name = "build-options",
                  .value = &build_options, .defval = 1 },
                { .type = 0, .short_name = 0, .long_name = 0, .value = 0, .defval = 0 }
            };
            printf("%d %d %s %d", options[0].type, options[0].short_name,
                   options[0].long_name, options[0].defval);
            *options[0].value = 1;
            printf(" %d %d", build_options, options[1].type);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 0 build-options 1 1 0");
}


// git parse-options OPT_BIT('w', ...): char constant into int short_name must
// store the full int field width. A 1-byte store leaves upper stack bytes
// garbage so 0x7F <= short_name and parse_options_check dies.
TEST(Compiler, localStructArrayDesignatedInitCharToIntField) {
    SourceProgram program{R"prg(
        struct Opt {
            int type;
            int short_name;
            const char *long_name;
            void *value;
            unsigned long precision;
            const char *argh;
            const char *help;
            int flags;
            void *callback;
            long defval;
        };
        int main() {
            int flags;
            const char *type;
            flags = 0;
            type = 0;
            struct Opt opts[] = {
                { .type = 10, .short_name = 't', .long_name = 0, .value = &type,
                  .argh = "type", .help = "object type", .flags = 0 },
                { .type = 5, .short_name = 'w', .long_name = 0, .value = &flags,
                  .precision = 4, .help = "write", .flags = 2, .callback = 0, .defval = 1 },
                { .type = 0 }
            };
            int bad0;
            int bad1;
            bad0 = (127 <= opts[0].short_name);
            bad1 = (127 <= opts[1].short_name);
            printf("%d %d %d %d %d %ld", opts[0].short_name, opts[1].short_name,
                   bad0, bad1, opts[1].flags, opts[1].defval);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("116 119 0 0 2 1");
}


TEST(Compiler, structDesignatedInitializerPointerMember) {
    SourceProgram program{R"prg(
        char slop;
        struct Buf {
            int alloc;
            int len;
            char *buf;
        };
        int main() {
            struct Buf b = { .buf = &slop };
            int eq;
            eq = (b.buf == &slop);
            printf("%d %d %d", b.alloc, b.len, eq);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1");
}


// git STRBUF_INIT: { .buf = strbuf_slopbuf } - array decays to pointer (no &).
TEST(Compiler, structDesignatedInitializerArrayDecay) {
    SourceProgram program{R"prg(
        char slop[1];
        struct Buf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        int main() {
            struct Buf b = { .buf = slop };
            int eq;
            eq = (b.buf == slop);
            printf("%lu %lu %d", b.alloc, b.len, eq);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1");
}


// File-scope STRBUF_INIT (git: static/global strbufs and STRBUF_INIT macros).
TEST(Compiler, globalStructDesignatedInitializerArrayDecay) {
    SourceProgram program{R"prg(
        char slop[1];
        struct Buf {
            unsigned long alloc;
            unsigned long len;
            char *buf;
        };
        struct Buf b = { .buf = slop };
        int main() {
            int eq;
            eq = (b.buf == slop);
            printf("%lu %lu %d", b.alloc, b.len, eq);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 0 1");
}


// git parse_event_data: local struct with pointer-to-struct member brace-init.
// Must store the pointer, not zero-expand the pointee (stack clobber of neighbors).
TEST(Compiler, localStructBraceInitPointerToStructMember) {
    SourceProgram program{R"prg(
        struct Big {
            long a;
            long b;
            long c;
            long d;
            long e;
            long f;
            long g;
            long h;
            long i;
            long j;
        };
        struct Ped {
            int t;
            long off;
            const struct Big *opts;
        };
        int main() {
            struct Big b;
            const struct Big *opts;
            long guard;
            b.a = 1;
            b.b = 2;
            b.c = 3;
            b.d = 4;
            b.e = 5;
            b.f = 6;
            b.g = 7;
            b.h = 8;
            b.i = 9;
            b.j = 10;
            opts = &b;
            guard = 4660;
            {
                struct Ped event_data = { 4, 0, opts };
                printf("%d %ld %ld %ld", event_data.t, event_data.off,
                        (long)event_data.opts, guard);
            }
            return 0;
        }
    )prg"};
    program.compile();
    // opts must be non-null (&b); guard must survive init (not clobbered).
    // Compare opts to &b via printing (long)opts - (long)&b == 0.
    SourceProgram program2{R"prg(
        struct Big {
            long a;
            long b;
            long c;
            long d;
            long e;
            long f;
            long g;
            long h;
            long i;
            long j;
        };
        struct Ped {
            int t;
            long off;
            const struct Big *opts;
        };
        int main() {
            struct Big b;
            const struct Big *opts;
            long guard;
            long diff;
            b.a = 1;
            opts = &b;
            guard = 4660;
            {
                struct Ped event_data = { 4, 0, opts };
                diff = (long)event_data.opts - (long)&b;
                printf("%d %ld %ld %ld", event_data.t, event_data.off, diff, guard);
            }
            return 0;
        }
    )prg"};
    program2.compile();
    program2.runAndExpect("4 0 0 4660");
}


// Local struct member char array from string (git STRBUF-like / color slots).
TEST(Compiler, localStructCharArrayStringInitializer) {
    SourceProgram program{R"prg(
        struct S {
            int relative;
            char c[8];
            int only;
        };
        int main() {
            struct S s = { .relative = 1, .c = "xy", .only = 0 };
            printf("%d %s %d %02x %02x", s.relative, s.c, s.only,
                (int)(unsigned char)s.c[0], (int)(unsigned char)s.c[1]);
            return 0;
        }
    )prg", "local_struct_char_str"};
    program.compile();
    program.runAndExpect("1 xy 0 78 79");
}


// git GREP_OPT_INIT: nested designated 2D char colors inside a local struct,
// then memcpy to the live object. Empty colors => no match highlighting (t4202).
TEST(Compiler, localStructNestedChar2DStringInitializer) {
    SourceProgram program{R"prg(
        struct grep_opt {
            int relative;
            int pathname;
            char colors[3][8];
            int only_matching;
        };
        void *memcpy(void *d, const void *s, unsigned long n);
        void *memset(void *d, int c, unsigned long n);
        void grep_init(struct grep_opt *opt) {
            struct grep_opt blank = {
                .relative = 1,
                .pathname = 1,
                .colors = {
                    [0] = "",
                    [1] = "\033[35m",
                    [2] = "\033[1;31m",
                },
                .only_matching = 0,
            };
            memcpy(opt, &blank, (unsigned long)sizeof(*opt));
        }
        int main() {
            struct grep_opt opt;
            memset(&opt, 0, (unsigned long)sizeof(opt));
            grep_init(&opt);
            printf("%d %d %d %02x %02x %02x %02x %02x %s",
                opt.relative, opt.pathname, opt.only_matching,
                (int)(unsigned char)opt.colors[1][0],
                (int)(unsigned char)opt.colors[1][1],
                (int)(unsigned char)opt.colors[1][2],
                (int)(unsigned char)opt.colors[2][0],
                (int)(unsigned char)opt.colors[2][1],
                opt.colors[1][0] ? "on" : "off");
            return 0;
        }
    )prg", "grep_opt_init_colors"};
    program.compile();
    // relative pathname only; magenta ESC [ 3; bold-red ESC [
    program.runAndExpect("1 1 0 1b 5b 33 1b 5b on");
}

} // namespace
