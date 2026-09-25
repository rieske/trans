#include "TestFixtures.h"

#include <cctype>
#include <sstream>
#include <string>

namespace {

std::string functionAssembly(const std::string& text, const char* name) {
    const std::string plain = std::string("\n") + name + ":";
    const std::string intel = std::string("\n$") + name + ":";
    const std::size_t plainAt = text.find(plain);
    const std::size_t intelAt = text.find(intel);
    std::size_t at = std::string::npos;
    if (plainAt == std::string::npos) {
        at = intelAt;
    } else if (intelAt == std::string::npos) {
        at = plainAt;
    } else {
        at = plainAt < intelAt ? plainAt : intelAt;
    }
    if (at == std::string::npos) {
        return {};
    }
    ++at;
    const std::size_t from = at;
    std::size_t end = text.size();
    for (const char* marker : { "\nmain:", "\n$main:", "\n.globl ", "\nglobal " }) {
        const std::size_t hit = text.find(marker, from);
        if (hit != std::string::npos && hit < end) {
            end = hit;
        }
    }
    return text.substr(at, end - at);
}

int countAdds(const std::string& body) {
    int count = 0;
    std::istringstream lines { body };
    std::string line;
    while (std::getline(lines, line)) {
        std::size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])) != 0) {
            ++i;
        }
        const std::string mnemonic = line.substr(i);
        if (mnemonic.compare(0, 4, "addl") == 0 || mnemonic.compare(0, 4, "addq") == 0
                || mnemonic.compare(0, 4, "add ") == 0) {
            ++count;
        }
    }
    return count;
}

void expectAdds(const char* source, const char* output, int atOpt0, int atOpt) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    const int adds = countAdds(functionAssembly(program.readAssembly(), "f"));
    EXPECT_EQ(adds, functionalTestOptFlag() == "-O0" ? atOpt0 : atOpt);
}

TEST(Compiler, reusesAddAcrossFallthroughLabel) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            x = a + b;
            goto L;
            L:
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 3, 2);
}

TEST(Compiler, keepsAddAcrossEmptyIf) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            int y;
            x = a + b;
            if (a) {}
            y = a + b;
            return x + y;
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 3, 3);
}

TEST(Compiler, keepsAddWhenOperandChanges) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            x = a + b;
            a = a + 1;
            goto L;
            L:
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "11", 4, 4);
}

TEST(Compiler, keepsVolatileAddAcrossFallthroughLabel) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(volatile int a, int b) {
            int x;
            x = a + b;
            goto L;
            L:
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 3, 3);
}

TEST(Compiler, keepsAddAcrossCall) {
    expectAdds(R"prg(int printf(const char *, ...);
        int g(int x) {
            int n;
            n = 1;
            char buf[n];
            buf[0] = 0;
            return buf[0];
        }
        int f(int a, int b) {
            int x;
            x = a + b;
            g(x);
            goto L;
            L:
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 3, 3);
}

void expectPrint(const char* source, const char* output) {
    SourceProgram program { source };
    program.compile();
    program.runAndExpect(output);
}

TEST(Compiler, signedDivideIsNotUnsignedDivide) {
    expectPrint(R"prg(int printf(const char *, ...);
        unsigned long udiv(unsigned long a, unsigned long b) {
            return a / b;
        }
        unsigned long f(long a, long b) {
            long s;
            unsigned long u;
            s = a / b;
            u = udiv(a, b);
            return (s + u) & 255;
        }
        int main(void) {
            printf("%d", (int)f(-5, 2));
            return 0;
        }
    )prg", "251");
}

TEST(Compiler, signedRemainderIsNotUnsignedRemainder) {
    expectPrint(R"prg(int printf(const char *, ...);
        unsigned long umod(unsigned long a, unsigned long b) {
            return a % b;
        }
        unsigned long f(long a, long b) {
            long s;
            unsigned long u;
            s = a % b;
            u = umod(a, b);
            return (s + u) & 255;
        }
        int main(void) {
            printf("%d", (int)f(-5, 2));
            return 0;
        }
    )prg", "0");
}

TEST(Compiler, arithmeticShiftIsNotLogicalShift) {
    expectPrint(R"prg(int printf(const char *, ...);
        unsigned long ushr(unsigned long a, unsigned long b) {
            return a >> b;
        }
        unsigned long h(long a, long b) {
            long s;
            unsigned long u;
            s = a >> b;
            u = ushr(a, b);
            return s ^ u;
        }
        int main(void) {
            printf("%lx", h(-8, 1));
            return 0;
        }
    )prg", "8000000000000000");
}

TEST(Compiler, compoundAssignThroughPointerAddsAgain) {
    expectPrint(R"prg(int printf(const char *, ...);
        int f(int *p, int a) {
            return (*p += a) + a;
        }
        int main(void) {
            int x;
            x = 10;
            printf("%d", f(&x, 3));
            return 0;
        }
    )prg", "16");
}

} // namespace
