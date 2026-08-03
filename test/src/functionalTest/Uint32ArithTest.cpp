#include "TestFixtures.h"

namespace {

TEST(Compiler, uint32AddWrapsMod32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned x;
            x = 0xffffffffu;
            x = x + 2u;
            printf("%u", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, uint32PlusEqualsWrapsMod32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned x;
            x = 0xffffffffu;
            x += 2u;
            printf("%u", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, charIncrementDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Pair { char a; char b; };
        int main(void) {
            struct Pair s;
            s.a = 1;
            s.b = 2;
            s.a++;
            ++s.b;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("2 3");
}

TEST(Compiler, shortDecrementDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        struct Pair { short a; short b; };
        int main(void) {
            struct Pair s;
            s.a = 10;
            s.b = 20;
            s.a--;
            --s.b;
            printf("%d %d", s.a, s.b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("9 19");
}

TEST(Compiler, uint32ArrayPlusEqualsDoesNotClobberNeighbor) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void add_iv(unsigned ihv[5], unsigned a) {
            ihv[0] += a;
        }
        int main(void) {
            unsigned ihv[5];
            ihv[0] = 0xffffffffu;
            ihv[1] = 0xefcdab89u;
            ihv[2] = 0x98badcfeu;
            add_iv(ihv, 2u);
            printf("%08x %08x %08x", ihv[0], ihv[1], ihv[2]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("00000001 efcdab89 98badcfe");
}

TEST(Compiler, uint32RotateLeftHighBit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned x;
            unsigned r;
            x = 0xefcdab89u;
            r = (x << 5) | (x >> 27);
            printf("%08x", r);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("f9b5713d");
}

TEST(Compiler, uint32BswapPattern) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned x;
            x = 0x12345678u;
            x = ((x << 8) & 0xff00ff00u) | ((x >> 8) & 0x00ff00ffu);
            x = (x << 16) | (x >> 16);
            printf("%08x", x);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("78563412");
}

TEST(Compiler, uint32ArrayLoadKeepsWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned ihv[5];
            unsigned a;
            unsigned b;
            ihv[0] = 0x67452301u;
            ihv[1] = 0xefcdab89u;
            a = ihv[0];
            b = ihv[1];
            a = (a << 5) | (a >> 27);
            b = (b << 30) | (b >> 2);
            printf("%08x %08x", a, b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("e8a4602c 7bf36ae2");
}

TEST(Compiler, uint32TwoDimParamWriteRead) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void store_state(unsigned states[80][5], int i, unsigned a, unsigned b) {
            states[i][0] = a;
            states[i][1] = b;
        }
        int main(void) {
            unsigned states[80][5];
            unsigned x;
            unsigned y;
            int i;
            for (i = 0; i < 80; i = i + 1) {
                states[i][0] = 0;
                states[i][1] = 0;
            }
            store_state(states, 0, 0x67452301u, 0xefcdab89u);
            store_state(states, 79, 0x10325476u, 0xc3d2e1f0u);
            x = states[0][0];
            y = states[79][1];
            printf("%08x %08x %08x", x, y, states[1][0]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("67452301 c3d2e1f0 00000000");
}

TEST(Compiler, uint32PtrHighBitRotate) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned words[2];
            unsigned *p;
            unsigned t;
            unsigned long w;
            words[0] = 0xefcdab89u;
            words[1] = 0x12345678u;
            p = words;
            t = p[0];
            w = t;
            t = (t << 5) | (t >> 27);
            printf("%08x %lx %08x", t, w, p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("f9b5713d efcdab89 12345678");
}

TEST(Compiler, uint32AddThenRotateUsesMod32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned step(unsigned e, unsigned addend) {
            e += addend;
            return (e << 5) | (e >> 27);
        }
        int main(void) {
            unsigned r;
            r = step(0xffffffffu, 0xffffffffu);
            printf("%08x", r);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ffffffdf");
}

TEST(Compiler, uint32AddThenRotateLocalUsesMod32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned e;
            e = 0xffffffffu;
            e += 0xffffffffu;
            e = (e << 5) | (e >> 27);
            printf("%08x", e);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ffffffdf");
}

TEST(Compiler, uint32Sha1StepMacroUsesMod32) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            unsigned a, b, c, d, e, m0, m1, t;
            a = 0x67452301u;
            b = 0xefcdab89u;
            c = 0x98badcfeu;
            d = 0x10325476u;
            e = 0xc3d2e1f0u;
            m0 = 0x626c6f62u;
            m1 = 0x20330068u;
            t = (d) ^ ((b) & ((c) ^ (d)));
            e += ((a << 5) | (a >> 27)) + t + 0x5A827999u + m0;
            b = (b << 30) | (b >> 2);
            t = (c) ^ ((a) & ((b) ^ (c)));
            d += ((e << 5) | (e >> 27)) + t + 0x5A827999u + m1;
            a = (a << 30) | (a >> 2);
            printf("%08x %08x", d, a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("cb04d015 59d148c0");
}

TEST(Compiler, sha1BlobHiMatchesGit) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        unsigned rotl(unsigned x, int n) {
            return (x << n) | (x >> (32 - n));
        }
        unsigned sha1_10(const unsigned char *p) {
            unsigned w[80];
            unsigned a, b, c, d, e, t, i, tmp;
            unsigned char blk[64];
            for (i = 0; i < 64; i = i + 1) {
                blk[i] = 0;
            }
            for (i = 0; i < 10; i = i + 1) {
                blk[i] = p[i];
            }
            blk[10] = 0x80;
            blk[63] = 80;
            for (i = 0; i < 16; i = i + 1) {
                w[i] = ((unsigned)blk[i * 4] << 24)
                    | ((unsigned)blk[i * 4 + 1] << 16)
                    | ((unsigned)blk[i * 4 + 2] << 8)
                    | (unsigned)blk[i * 4 + 3];
            }
            for (i = 16; i < 80; i = i + 1) {
                w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }
            a = 0x67452301u;
            b = 0xefcdab89u;
            c = 0x98badcfeu;
            d = 0x10325476u;
            e = 0xc3d2e1f0u;
            for (i = 0; i < 80; i = i + 1) {
                if (i < 20) {
                    t = (b & c) | ((~b) & d);
                    tmp = 0x5a827999u;
                } else {
                    if (i < 40) {
                        t = b ^ c ^ d;
                        tmp = 0x6ed9eba1u;
                    } else {
                        if (i < 60) {
                            t = (b & c) | (b & d) | (c & d);
                            tmp = 0x8f1bbcdcu;
                        } else {
                            t = b ^ c ^ d;
                            tmp = 0xca62c1d6u;
                        }
                    }
                }
                tmp = rotl(a, 5) + t + e + tmp + w[i];
                e = d;
                d = c;
                c = rotl(b, 30);
                b = a;
                a = tmp;
            }
            a = a + 0x67452301u;
            b = b + 0xefcdab89u;
            c = c + 0x98badcfeu;
            d = d + 0x10325476u;
            e = e + 0xc3d2e1f0u;
            printf("%08x%08x%08x%08x%08x", a, b, c, d, e);
            return a;
        }
        int main(void) {
            unsigned char msg[10];
            msg[0] = 98;
            msg[1] = 108;
            msg[2] = 111;
            msg[3] = 98;
            msg[4] = 32;
            msg[5] = 51;
            msg[6] = 0;
            msg[7] = 104;
            msg[8] = 105;
            msg[9] = 10;
            sha1_10(msg);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("45b983be36b73c0788dc9cbcb76cbb80fc7bb057");
}

// int -> size_t must zero-extend. Dirty high 32 bits in realloc's size_t make
// glibc malloc-debug (git's test harness) die with "Out of memory, realloc failed".
TEST(Compiler, intAssignedToSizeTIsZeroExtended) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *realloc(void *, unsigned long);
        void free(void *);
        int main(void) {
            int n;
            unsigned long s;
            void *p;
            n = 7;
            s = n;
            p = realloc(0, s);
            printf("%d", p ? 1 : 0);
            free(p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

TEST(Compiler, reallocTakesIntSizeZeroExtended) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *realloc(void *, unsigned long);
        void free(void *);
        int main(void) {
            int n;
            void *p;
            n = 7;
            p = realloc(0, n);
            printf("%d", p ? 1 : 0);
            free(p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// git ALLOC_GROW: int alloc, then realloc(p, (size_t)n * sizeof(*p)).
TEST(Compiler, allocGrowIntCountTimesPointerSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *realloc(void *, unsigned long);
        void free(void *);
        struct Item { int x; };
        int main(void) {
            struct Item **items;
            int nr;
            int alloc;
            int n;
            items = 0;
            nr = 0;
            alloc = 0;
            n = alloc + 16;
            n = (n * 3) / 2;
            items = realloc(items, (unsigned long)n * sizeof(struct Item *));
            alloc = n;
            items[0] = 0;
            nr = 1;
            printf("%d %d", alloc, items ? 1 : 0);
            free(items);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("24 1");
}

// size_t++ must be a 64-bit register inc + 8-byte store, not incl on memory.
TEST(Compiler, sizeTIncrementThenReallocIsFullWidth) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *realloc(void *, unsigned long);
        void free(void *);
        int main(void) {
            unsigned long s;
            void *p;
            s = 6;
            s++;
            p = realloc(0, s);
            printf("%d", p ? 1 : 0);
            free(p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

// Signed W32 ~0 used as long is -1, not zero-extended 0xffffffff.
TEST(Compiler, signedIntBitwiseNotAsLongIsMinusOne) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            long y;
            x = 0;
            y = ~x;
            printf("%ld", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, signedIntDivAsPointerOffset) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char buf[4];
            int n;
            char *p;
            buf[0] = 65;
            buf[1] = 66;
            buf[2] = 67;
            buf[3] = 0;
            n = -2;
            p = buf + 3;
            p = p + (n / 1);
            printf("%c", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("B");
}

TEST(Compiler, signedIntModAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int a;
            int b;
            long y;
            a = -5;
            b = 3;
            y = a % b;
            printf("%ld", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-2");
}

TEST(Compiler, signedIntSarAsLong) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int x;
            long y;
            x = -2;
            y = x >> 1;
            printf("%ld", y);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("-1");
}

TEST(Compiler, signedIntIncFromMinusTwoAsOffset) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            char buf[3];
            int n;
            char *p;
            buf[0] = 65;
            buf[1] = 66;
            buf[2] = 0;
            n = -2;
            n++;
            p = buf + 2;
            p = p + n;
            printf("%c", *p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("B");
}

TEST(Compiler, signedIntCompareAndZeroCompare) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main(void) {
            int a;
            int b;
            a = -1;
            b = -1;
            printf("%d %d", a == b, a ? 1 : 0);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1 1");
}

// git worktree_basename: size is path + len - name (ptrdiff, then size_t).
TEST(Compiler, pointerDiffAsReallocSize) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        void *realloc(void *, unsigned long);
        void free(void *);
        int main(void) {
            const char *path;
            const char *name;
            int len;
            void *p;
            path = "linked-worktree";
            name = path;
            len = 7;
            p = realloc(0, path + len - name);
            printf("%d", p ? 1 : 0);
            free(p);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("1");
}

} // namespace
