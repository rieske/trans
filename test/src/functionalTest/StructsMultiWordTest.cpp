#include "TestFixtures.h"

namespace {


// Multi-word struct assignment through a pointer must copy every field.
TEST(Compiler, structAssignThroughPointer) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        int main() {
            struct S s;
            struct S t;
            struct S *p;
            s.a = 1;
            s.b = 2;
            t.a = 7;
            t.b = 9;
            p = &s;
            *p = t;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("7 9");
}


// Multi-word member rvalue must load every word (not just the first 8 bytes).
// git: r->event_jobs = cb_data.event_jobs (strmap ~64B); partial load left
// tablesize/cmpfn as stack garbage and SEGV'd in hashmap_get.
TEST(Compiler, multiWordMemberAssignCopiesAllWords) {
    SourceProgram program{R"prg(
        struct Big {
            long a;
            long b;
            long c;
            long d;
        };
        struct Holder {
            int tag;
            struct Big payload;
        };
        int main() {
            struct Holder src;
            struct Holder dst;
            struct Holder *p;
            src.tag = 1;
            src.payload.a = 10;
            src.payload.b = 20;
            src.payload.c = 30;
            src.payload.d = 40;
            dst.tag = 0;
            dst.payload.a = 0;
            dst.payload.b = 0;
            dst.payload.c = 0;
            dst.payload.d = 0;
            dst.payload = src.payload;
            printf("%ld %ld %ld %ld", dst.payload.a, dst.payload.b, dst.payload.c, dst.payload.d);
            p = &dst;
            p->payload.a = 0;
            p->payload.b = 0;
            p->payload.c = 0;
            p->payload.d = 0;
            p->payload = src.payload;
            printf(" %ld %ld %ld %ld", p->payload.a, p->payload.b, p->payload.c, p->payload.d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("10 20 30 40 10 20 30 40");
}


// Same bug via pointer dereference of a multi-word struct (*p = *q).
TEST(Compiler, multiWordStructDerefAssignCopiesAllWords) {
    SourceProgram program{R"prg(
        struct Big {
            long a;
            long b;
            long c;
            long d;
        };
        int main() {
            struct Big s;
            struct Big t;
            struct Big *ps;
            struct Big *pt;
            s.a = 1;
            s.b = 2;
            s.c = 3;
            s.d = 4;
            t.a = 0;
            t.b = 0;
            t.c = 0;
            t.d = 0;
            ps = &s;
            pt = &t;
            *pt = *ps;
            printf("%ld %ld %ld %ld", t.a, t.b, t.c, t.d);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 3 4");
}


// git object_id is 36 bytes (hash[32] + algo); array stride is 36, not 40.
// Multi-word assign must not round the last store up to a full qword or it
// clobbers the next element (khash keys[j] = key / oidset insert).
TEST(Compiler, multiWordStructArrayAssignDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(
        struct object_id {
            unsigned char hash[32];
            unsigned int algo;
        };
        struct object_id keys[4];
        int main() {
            struct object_id k;
            int i;
            for (i = 0; i < 4; i = i + 1) {
                keys[i].hash[0] = 0;
                keys[i].algo = 0;
            }
            for (i = 0; i < 32; i = i + 1) {
                k.hash[i] = 0;
            }
            k.hash[0] = 2;
            k.algo = 1;
            keys[1] = k;
            k.hash[0] = 9;
            keys[0] = k;
            printf("%u %u %u", keys[0].hash[0], keys[1].hash[0], keys[1].algo);
            return 0;
        }
    )prg", "oid_array_stride"};
    program.compile();
    program.runAndExpect("9 2 1");
}


// Same overrun via *p = value into a packed struct array (khash put path).
TEST(Compiler, multiWordStructPtrAssignDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(
        struct object_id {
            unsigned char hash[32];
            unsigned int algo;
        };
        int main() {
            struct object_id keys[4];
            struct object_id k;
            struct object_id *p;
            int i;
            for (i = 0; i < 4; i = i + 1) {
                keys[i].hash[0] = 0;
                keys[i].algo = 0;
            }
            for (i = 0; i < 32; i = i + 1) {
                k.hash[i] = 0;
            }
            k.hash[0] = 2;
            k.algo = 1;
            p = &keys[1];
            *p = k;
            k.hash[0] = 9;
            p = &keys[0];
            *p = k;
            printf("%u %u %u", keys[0].hash[0], keys[1].hash[0], keys[1].algo);
            return 0;
        }
    )prg", "oid_ptr_stride"};
    program.compile();
    program.runAndExpect("9 2 1");
}


// git strbuf_init: multi-word local (struct blank) plus register args must not share
// stack slots. Bug was localIndex = scopeValues.size() (entry count) instead of the
// next free word index, so hint's home collided with blank and memcpy's spills
// clobbered the saved hint - strbuf_grow(sb, (size_t)sb) / huge xrealloc.
TEST(Compiler, multiWordLocalDoesNotClobberRegisterArgs) {
    SourceProgram program{R"prg(
        struct Triple {
            int a;
            int b;
            int c;
        };
        int grows;
        int last_hint;
        void record_grow(struct Triple *sb, int hint) {
            grows = grows + 1;
            last_hint = hint;
        }
        void init_like_strbuf(struct Triple *sb, int hint) {
            struct Triple blank;
            blank.a = 0;
            blank.b = 0;
            blank.c = 0;
            sb->a = blank.a;
            sb->b = blank.b;
            sb->c = blank.c;
            if (hint) {
                record_grow(sb, hint);
            }
        }
        int main() {
            struct Triple sb;
            grows = 0;
            last_hint = 99;
            init_like_strbuf(&sb, 0);
            printf("%d %d", grows, last_hint);
            init_like_strbuf(&sb, 5);
            printf(" %d %d", grows, last_hint);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 99 1 5");
}


// Multi-word file-scope structs must reserve full size in .data (not a single
// dq). Otherwise a following global shares the same storage and is clobbered
// when a high-offset member is written (git: static struct repository the_repo).
TEST(Compiler, globalStructReservesFullSize) {
    SourceProgram program{R"prg(
        struct Pair {
            int a;
            int b;
        };

        struct Pair g;
        int adjacent;

        int main() {
            adjacent = 42;
            g.a = 1;
            g.b = 7;
            printf("%d %d %d", g.a, g.b, adjacent);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 7 42");
}


// Packed int members must not clobber each other (System V layout, 4-byte stores).
TEST(Compiler, packedIntStructMembersIndependent) {
    SourceProgram program{R"prg(
        struct S {
            int a;
            int b;
        };
        int main() {
            struct S s;
            s.a = 1;
            s.b = 2;
            printf("%d %d %d", s.a, s.b, (int)sizeof(s));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 2 8");
}


// fstat fills libc layout; st_size must be read at the ABI offset (git hash-object).
TEST(Compiler, fstatStSizeMatchesFileLength) {
    SourceProgram program{R"prg(
        struct stat {
            unsigned long st_dev;
            unsigned long st_ino;
            unsigned long st_nlink;
            unsigned int st_mode;
            unsigned int st_uid;
            unsigned int st_gid;
            int __pad0;
            unsigned long st_rdev;
            long st_size;
            long st_blksize;
            long st_blocks;
            long st_atim_sec;
            long st_atim_nsec;
            long st_mtim_sec;
            long st_mtim_nsec;
            long st_ctim_sec;
            long st_ctim_nsec;
            long __glibc_reserved[3];
        };
        int open(const char *p, int f, int m);
        int write(int fd, const void *b, unsigned long n);
        int fstat(int fd, struct stat *st);
        int close(int fd);
        int main() {
            struct stat st;
            int fd;
            fd = open("/tmp/trans-stat-size.txt", 577, 420);
            if (fd < 0) {
                printf("openw");
                return 0;
            }
            write(fd, "abcdef", 6);
            close(fd);
            fd = open("/tmp/trans-stat-size.txt", 0, 0);
            if (fd < 0) {
                printf("openr");
                return 0;
            }
            if (fstat(fd, &st) != 0) {
                printf("fstat");
                return 0;
            }
            close(fd);
            printf("%ld", st.st_size);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}


// Large static struct (the_repo pattern): high member offsets must not corrupt
// a pointer that immediately follows the object in .data.
TEST(Compiler, staticLargeGlobalStructPreservesFollowingPointer) {
    SourceProgram program{R"prg(
        struct Big {
            int a;
            int b;
            int c;
            int d;
            int e;
            int f;
            int g;
            int h;
        };

        static struct Big the_big;
        struct Big *the_ptr;
        int adjacent;

        int main() {
            the_ptr = &the_big;
            adjacent = 99;
            the_ptr->a = 1;
            the_ptr->h = 8;
            printf("%d %d %d %d", the_big.a, the_big.h, adjacent, the_ptr == &the_big);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 8 99 1");
}


// Same pattern with memcpy (git strbuf_init copies STRBUF_INIT then maybe grow).
TEST(Compiler, multiWordLocalMemcpyPreservesHintArg) {
    SourceProgram program{R"prg(
        void *memcpy(void *dest, const void *src, unsigned long n);
        struct Triple {
            int a;
            int b;
            int c;
        };
        int grows;
        int last_hint;
        void record_grow(struct Triple *sb, int hint) {
            grows = grows + 1;
            last_hint = hint;
        }
        void init_like_strbuf(struct Triple *sb, int hint) {
            struct Triple blank;
            blank.a = 0;
            blank.b = 0;
            blank.c = 0;
            memcpy(sb, &blank, 24);
            if (hint) {
                record_grow(sb, hint);
            }
        }
        int main() {
            struct Triple sb;
            grows = 0;
            last_hint = 99;
            sb.a = 1;
            sb.b = 2;
            sb.c = 3;
            init_like_strbuf(&sb, 0);
            printf("%d %d %d %d %d", grows, last_hint, sb.a, sb.b, sb.c);
            init_like_strbuf(&sb, 5);
            printf(" %d %d", grows, last_hint);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 99 0 0 0 1 5");
}


// git repack.c: struct generated_pack { tempfile *tempfiles[ARRAY_SIZE(exts)]; }
// ARRAY_SIZE is sizeof(exts)/sizeof(exts[0]). If the bound is not folded when the
// struct member type is built, the member decays to a single pointer (sizeof 8),
// xcalloc under-allocates, and pack->tempfiles[i] = ... SEGV (loads *pack as base).
// Use sizeof(*p) / sizeof(local) - same as git's sizeof(*pack) - not sizeof(type_name),
// which is folded at AST-build before the member bound fixup runs.
TEST(Compiler, structMemberArraySizeofBoundIsArrayNotPointer) {
    SourceProgram program{R"prg(
        struct E {
            long x;
            long y;
        };
        static struct E exts[] = {
            { 1, 0 },
            { 2, 1 },
            { 3, 1 },
            { 4, 1 },
            { 5, 1 },
            { 6, 0 }
        };
        struct Pack {
            long *tempfiles[(sizeof(exts) / sizeof((exts)[0]))];
        };
        void *malloc(unsigned long);
        int main() {
            struct Pack local;
            struct Pack *p;
            int i;
            printf("%d ", (int)sizeof(local));
            p = (struct Pack *)malloc((unsigned long)sizeof(*p));
            i = 0;
            while (i < 6) {
                p->tempfiles[i] = 0;
                i = i + 1;
            }
            p->tempfiles[0] = &exts[0].x;
            p->tempfiles[5] = &exts[5].x;
            printf("%ld %ld", *(p->tempfiles[0]), *(p->tempfiles[5]));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("48 1 6");
}


// Assign through * on a char[1] member (git ustar_header: *header.typeflag = TYPEFLAG_REG).
// The member result temp must hold a full pointer; sizing it as char[1] made emitLoad
// sign-extend one byte of the address and SEGV in git archive --format=tar.
TEST(Compiler, assignThroughStarOnCharArrayMember) {
    SourceProgram program{R"prg(
        struct ustar {
            char name[100];
            char mode[8];
            char uid[8];
            char gid[8];
            char size[12];
            char mtime[12];
            char chksum[8];
            char typeflag[1];
            char linkname[100];
        };
        int main() {
            struct ustar h;
            *h.typeflag = 48;
            printf("%c %d", h.typeflag[0], (int)*h.typeflag);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("0 48");
}


// Unary * on a struct char-array member must load the first byte (*s.str == s.str[0]).
// git interned_mode_string: if (!*mode_string[i].str) snprintf(...).
// Member-array result already holds &s.str[0]; * must not AddressOf that temporary.
TEST(Compiler, unaryStarOnStructCharArrayMember) {
    SourceProgram program{R"prg(
        struct S {
            unsigned int val;
            char str[7];
        };
        static struct S a = { .val = 0040000 };
        int main() {
            a.str[0] = 0;
            a.str[1] = 0;
            printf("%d %d %d",
                (int)(unsigned char)a.str[0],
                (int)(unsigned char)*a.str,
                !*a.str);
            a.str[0] = 'x';
            printf(" %d %d",
                (int)(unsigned char)*a.str,
                !*a.str);
            return 0;
        }
    )prg"};
    program.compile();
    // empty: index0=0, *str=0, !*str=1; after 'x': *str=120, !*str=0
    program.runAndExpect("0 0 1 120 0");
}


// git-style: pointer members coexist with ARRAY_SIZE array members; sizeof must
// include both (pointer member must not block re-fold of the array bound).
TEST(Compiler, structMemberArraySizeofBoundWithPointerSibling) {
    SourceProgram program{R"prg(
        struct E {
            long x;
            long y;
        };
        static struct E exts[] = {
            { 10, 0 },
            { 20, 0 },
            { 30, 0 }
        };
        struct Pack {
            long *extra;
            long *tempfiles[(sizeof(exts) / sizeof((exts)[0]))];
        };
        void *malloc(unsigned long);
        int main() {
            struct Pack local;
            struct Pack *p;
            long v;
            printf("%d ", (int)sizeof(local));
            p = (struct Pack *)malloc((unsigned long)sizeof(*p));
            p->extra = &exts[1].x;
            p->tempfiles[0] = &exts[0].x;
            p->tempfiles[2] = &exts[2].x;
            v = *(p->extra);
            printf("%ld %ld %ld", *(p->tempfiles[0]), v, *(p->tempfiles[2]));
            return 0;
        }
    )prg"};
    program.compile();
    // 1 pointer + 3 pointer array = 4 * 8 = 32
    program.runAndExpect("32 10 20 30");
}

} // namespace
