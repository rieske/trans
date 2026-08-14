#include "TestFixtures.h"

namespace {

// C 6.3.1.1 / 6.5.7 / 6.5.3.3: << >> + - ~ promote a narrow operand to int
// and the result has that promoted type. Values below differ if the op is
// performed at the unpromoted width (char 8, short 16).
TEST(Compiler, integerPromotionsWidenUnaryAndShiftOfNarrowTypes) {
    SourceProgram program{R"prg(int printf(const char *, ...);
        int main() {
            char c;
            unsigned char uc;
            short s;
            unsigned short us;
            unsigned char b0;
            unsigned char b1;
            unsigned char b2;
            unsigned char b3;
            unsigned pack;
            c = 1;
            uc = 1;
            s = 1;
            us = 1;
            printf("%d %d %d %d ", c << 8, uc << 8, s << 16, us << 16);
            c = 0x54;
            printf("%d ", (c << 24) == 0x54000000);
            b0 = 0x54;
            b1 = 0x52;
            b2 = 0x45;
            b3 = 0x45;
            pack = (unsigned)((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
            printf("%d ", pack == 0x54524545u);
            uc = 128;
            printf("%d %d ", +uc, (int)sizeof(+uc));
            uc = 0;
            us = 0;
            printf("%d %d ", ~uc, ~us);
            uc = 1;
            us = 1;
            printf("%d %d", -uc, -us);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("256 256 65536 65536 1 1 128 4 -1 -1 -1 -1");
}

} // namespace
