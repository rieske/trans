#include "TestFixtures.h"

namespace {

TEST(Compiler, shiftLeft) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d ", a << 1);
            printf("%d ", a << 2);
            printf("%d", a << 3);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("1", "2 4 8");
    program.runAndExpect("2", "4 8 16");
}

// Integer promotions on shift (C 6.5.7 / 6.3.1.1): char << n yields int, not char.
// git CACHE_EXT(s) = (s[0]<<24)|(s[1]<<16)|(s[2]<<8)|(s[3]); without promotion
// the result collapses to s[3] and index TREE extensions are ignored.
TEST(Compiler, charShiftLeftPromotesToInt) {
    SourceProgram program{R"prg(
        int main() {
            const char *s;
            unsigned v;
            s = "TREE";
            v = (unsigned)((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | (s[3]));
            printf("%u", v);
            return 0;
        }
    )prg"};
    program.compile();
    // 0x54524545 == 1414677829
    program.runAndExpect("", "1414677829");
}

TEST(Compiler, charShiftMatchesFourccTree) {
    SourceProgram program{R"prg(
        int main() {
            const char *ext;
            unsigned v;
            ext = "TREE";
            v = (unsigned)((ext[0] << 24) | (ext[1] << 16) | (ext[2] << 8) | (ext[3]));
            if (v == 0x54524545u) {
                printf("matched");
            } else {
                printf("bad %u", v);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("", "matched");
}

TEST(Compiler, shiftRight) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            scanf("%ld", &a);
            printf("%d ", a >> 1);
            printf("%d ", a >> 2);
            printf("%d", a >> 3);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("0", "0 0 0");
    program.runAndExpect("8", "4 2 1");
    program.runAndExpect("16", "8 4 2");
    // Signed arithmetic shift (SAR): high bits filled with sign bit.
    program.runAndExpect("-8", "-4 -2 -1");
    program.runAndExpect("-1", "-1 -1 -1");
}

// C 6.5.7: unsigned >> is logical (zero-fill). Emitting SAR for every >> makes
// UINTMAX_MAX >> n stay all-ones, so git parse-options upper bounds for
// uint16_t become UINTMAX_MAX instead of 65535 and --u16 65536 is accepted.
TEST(Compiler, unsignedRightShiftIsLogical) {
    SourceProgram program{R"prg(
        int main() {
            unsigned long long x;
            unsigned long long ub;
            x = 0xffffffffffffffffULL;
            ub = x >> 48;
            printf("%llu", ub);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("65535");
}

// Same formula as git parse-options OPTION_UNSIGNED upper_bound for 2-byte precision.
TEST(Compiler, unsignedMaxShiftYieldsUint16Max) {
    SourceProgram program{R"prg(
        typedef unsigned long long uintmax_t;
        int main() {
            uintmax_t upper;
            upper = 0xffffffffffffffffULL >> (64 - 8 * 2);
            if (upper == 65535ULL) {
                printf("ok");
            } else {
                printf("bad %llu", upper);
            }
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("ok");
}

// uint32_t values loaded from arrays must zero-extend; sign-extension breaks
// rotates used by SHA-1 (rol(0xefcdab89, 30) must be 0x7bf36ae2).
TEST(Compiler, unsignedIntRotateFromArray) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int h[2];
            unsigned int b;
            unsigned int r;
            h[0] = 1u;
            h[1] = 4023233417u;
            b = h[1];
            r = (b << 30) | (b >> 2);
            printf("%u", r);
            return 0;
        }
    )prg"};

    program.compile();

    // 0x7bf36ae2
    program.runAndExpect("", "2079550178");
}

// After 32-bit adds, high bits must not pollute a subsequent rotate.
TEST(Compiler, unsignedIntAddThenRotate) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int a;
            unsigned int b;
            unsigned int t;
            unsigned int c;
            a = 1732584193u;
            b = 4023233417u;
            t = a + b;
            c = (t << 30) | (t >> 2);
            printf("%u", c);
            return 0;
        }
    )prg"};

    program.compile();

    // t = 0x67452301 + 0xefcdab89 = 0x5712ce8a; rol(t, 30) = 0x95c4b3a2
    program.runAndExpect("", "2512696226");
}

// Compound += must wrap at 32 bits; otherwise high bits poison a later rotate
// (git sha1dc: e += rotate_left(a,5) + f + K + W[t]).
TEST(Compiler, unsignedIntAddAssignThenRotate) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int x;
            unsigned int r;
            x = 4294967295u;
            x += 1u;
            r = (x << 5) | (x >> 27);
            printf("%u %u", x, r);
            return 0;
        }
    )prg"};

    program.compile();

    // x wraps to 0; rol(0, 5) == 0
    program.runAndExpect("", "0 0");
}

// SHA1DC-style step: e += rol(a,5) + f + K + w with uint32 wrap, then rol(b,30).
TEST(Compiler, unsignedIntSha1StepCompoundAdd) {
    SourceProgram program{R"prg(
        int main() {
            unsigned int a;
            unsigned int b;
            unsigned int c;
            unsigned int d;
            unsigned int e;
            unsigned int w0;
            unsigned int f;
            a = 1732584193u;
            b = 4023233417u;
            c = 2562383102u;
            d = 271733878u;
            e = 3285377520u;
            w0 = 2147483648u;
            f = d ^ (b & (c ^ d));
            e += ((a << 5) | (a >> 27)) + f + 1518500249u + w0;
            b = (b << 30) | (b >> 2);
            printf("%u %u %u %u %u", a, b, c, d, e);
            return 0;
        }
    )prg"};

    program.compile();

    // After empty-message step0: a unchanged, b=rol(b,30)=2079550178,
    // c,d unchanged, e=0x1fb498b3
    program.runAndExpect("", "1732584193 2079550178 2562383102 271733878 531929267");
}

// Unsigned function parameters must zero-extend on load, not sign-extend.
// If signedness is lost when building the procedure frame (default isSigned=true),
// x>>n becomes arithmetic and SHA-256 ror/gamma1 on 0x80000000 go wrong
// (git sha256/block/sha256.c empty-message digest).
TEST(Compiler, unsignedIntParamRotateRightHighBit) {
    SourceProgram program{R"prg(
        typedef unsigned int uint32_t;
        static uint32_t ror(uint32_t x, unsigned n) {
            return (x >> n) | (x << (32 - n));
        }
        static uint32_t gamma1(uint32_t x) {
            return ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
        }
        int main() {
            uint32_t x = 0x80000000u;
            printf("%08x %08x %08x", ror(x, 17), ror(x, 19), gamma1(x));
            return 0;
        }
    )prg", "u32_param_ror"};
    program.compile();
    // ror(0x80000000,17)=0x4000; ror(...,19)=0x1000; gamma1=0x00205000
    program.runAndExpect("00004000 00001000 00205000");
}

// Empty SHA-256 block schedule W[18] depends on gamma1(0x80000000).
TEST(Compiler, unsignedIntSha256Gamma1MessageSchedule) {
    SourceProgram program{R"prg(
        typedef unsigned int uint32_t;
        static uint32_t ror(uint32_t x, unsigned n) {
            return (x >> n) | (x << (32 - n));
        }
        static uint32_t gamma0(uint32_t x) {
            return ror(x, 7) ^ ror(x, 18) ^ (x >> 3);
        }
        static uint32_t gamma1(uint32_t x) {
            return ror(x, 17) ^ ror(x, 19) ^ (x >> 10);
        }
        int main() {
            uint32_t W[64];
            int i;
            for (i = 0; i < 64; i = i + 1) {
                W[i] = 0;
            }
            W[0] = 0x80000000u;
            for (i = 16; i < 64; i = i + 1) {
                W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
            }
            printf("%08x %08x %08x %08x", W[16], W[17], W[18], W[63]);
            return 0;
        }
    )prg", "sha256_sched"};
    program.compile();
    program.runAndExpect("80000000 00000000 00205000 3b5ec49b");
}

} // namespace

