#include "TestFixtures.h"

namespace {

TEST(Compiler, longVariableArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            long a;
            long b;
            a = 100;
            b = 23;
            printf("%d", a + b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("123");
}

TEST(Compiler, unsignedVariableArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned a;
            unsigned b;
            a = 10;
            b = 7;
            printf("%d", a - b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, shortVariableArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            short a;
            short b;
            a = 4;
            b = 5;
            printf("%d", a * b);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("20");
}

TEST(Compiler, signedVariableArithmetic) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            signed a;
            a = -3;
            printf("%d", -a);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, longFunctionParameterAndReturn) {
    SourceProgram program{R"prg(#include <stdio.h>
        long add(long x, long y) {
            return x + y;
        }
        int main() {
            printf("%d", add(40, 2));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, unsignedPointer) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned v;
            unsigned* p;
            v = 99;
            p = &v;
            *p = 11;
            printf("%d", v);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, unsignedIntIsUnsigned) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned int a;
            a = 5;
            printf("%d", a + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("6");
}

TEST(Compiler, sizeofLongAndShort) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d %d", (int)sizeof(long), (int)sizeof(short), (int)sizeof(unsigned));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 2 4");
}

TEST(Compiler, multiWordUnsignedLongLocal) {
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            unsigned long x;
            x = 41;
            printf("%d", x + 1);
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("42");
}

TEST(Compiler, sizeofLongUnsignedOrderIndependent) {
    // type_name combine must not drop long when keywords are reordered.
    SourceProgram program{R"prg(#include <stdio.h>
        int main() {
            printf("%d %d", (int)sizeof(long unsigned), (int)sizeof(long unsigned int));
            return 0;
        }
    )prg"};
    program.compile();
    program.runAndExpect("8 8");
}

} // namespace
