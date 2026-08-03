#include "TestFixtures.h"

namespace {

TEST(Compiler, memberCharArrayPlusIndex) {
    SourceProgram program{R"prg(#include <stdio.h>
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
        };
        int main() {
            struct Ctx c;
            int left;
            unsigned char *p;
            c.buffer[0] = 65;
            c.buffer[1] = 66;
            c.buffer[2] = 67;
            c.buffer[3] = 68;
            left = 2;
            p = c.buffer + left;
            printf("%c%c", p[0], p[1]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("CD");
}

// Direct call arg: memcpy(ctx->buffer + left, src, n) must pass the pointer
// value, not &stack_temp of a phantom array result. Without pointer-arithmetic
// recognition for member arrays (result is already decayed pointer, type stays
// array for sizeof), + yields an array temporary and call-arg decay takes its
// address - SHA1DCUpdate never writes the real buffer (wrong empty hash).
TEST(Compiler, memcpyIntoMemberArrayPlusOffset) {
    SourceProgram program{R"prg(#include <stdio.h>
        void *memcpy(void *d, const void *s, unsigned long n);
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
            int flag;
        };
        void update(struct Ctx *ctx, const char *buf, unsigned len) {
            unsigned left;
            left = (unsigned)(ctx->total & 63);
            if (len > 0) {
                ctx->total = ctx->total + (unsigned long)len;
                memcpy(ctx->buffer + left, buf, (unsigned long)len);
            }
        }
        int main() {
            struct Ctx c;
            char ch;
            int i;
            for (i = 0; i < 64; i = i + 1) {
                c.buffer[i] = 0xAA;
            }
            c.total = 0;
            ch = (char)0x80;
            update(&c, &ch, 1);
            printf("%d %d", (int)c.buffer[0], (int)c.total);
            c.total = 3;
            c.buffer[0] = 1;
            c.buffer[1] = 2;
            c.buffer[2] = 3;
            update(&c, "XY", 2);
            printf(" %d %d %d %d %d %d", (int)c.buffer[0], (int)c.buffer[1],
                    (int)c.buffer[2], (int)c.buffer[3], (int)c.buffer[4],
                    (int)c.total);
            return 0;
        }
    )prg"};
    program.compile();
    // 128 1 from first update; then 1 2 3 'X' 'Y' total 5
    program.runAndExpect("128 1 1 2 3 88 89 5");
}

// End-to-end: partial fill of struct char buffer via pointer arithmetic + cast
// of a static padding array (SHA1DCFinal shape).
TEST(Compiler, sha1PaddingCastAndBufferOffset) {
    SourceProgram program{R"prg(#include <stdio.h>
        static unsigned char pad[64];
        struct Ctx {
            unsigned long total;
            unsigned char buffer[64];
        };
        void fill(struct Ctx *ctx, const char *buf, int len) {
            int left;
            left = (int)(ctx->total & 63);
            if (len > 0) {
                int i;
                unsigned char *dst;
                dst = ctx->buffer + left;
                for (i = 0; i < len; i = i + 1) {
                    dst[i] = (unsigned char)buf[i];
                }
                ctx->total = ctx->total + (unsigned long)len;
            }
        }
        int main() {
            struct Ctx c;
            int i;
            c.total = 0;
            for (i = 0; i < 64; i = i + 1) {
                c.buffer[i] = 0;
            }
            pad[0] = 128;
            pad[1] = 0;
            pad[2] = 0;
            fill(&c, "hi", 2);
            fill(&c, (const char *)(pad), 3);
            printf("%d %d %d %d %d", (int)c.buffer[0], (int)c.buffer[1],
                    (int)c.buffer[2], (int)c.buffer[3], (int)c.buffer[4]);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("104 105 128 0 0");
}

} // namespace
