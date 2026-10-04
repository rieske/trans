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

int countCmps(const std::string& body) {
    int count = 0;
    std::istringstream lines { body };
    std::string line;
    while (std::getline(lines, line)) {
        std::size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])) != 0) {
            ++i;
        }
        const std::string mnemonic = line.substr(i);
        if (mnemonic.compare(0, 3, "cmp") == 0 && mnemonic.compare(0, 4, "cmps") != 0
                && mnemonic.compare(0, 4, "cmpx") != 0) {
            ++count;
        }
    }
    return count;
}

void expectCmps(const char* source, const char* output, int atOpt0, int atOpt) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    const int cmps = countCmps(functionAssembly(program.readAssembly(), "f"));
    EXPECT_EQ(cmps, functionalTestOptFlag() == "-O0" ? atOpt0 : atOpt);
}

int countSubs(const std::string& body) {
    int count = 0;
    std::istringstream lines { body };
    std::string line;
    while (std::getline(lines, line)) {
        std::size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])) != 0) {
            ++i;
        }
        const std::string mnemonic = line.substr(i);
        if (mnemonic.find("rsp") != std::string::npos || mnemonic.find("esp") != std::string::npos) {
            continue;
        }
        if (mnemonic.compare(0, 4, "subl") == 0 || mnemonic.compare(0, 4, "sub ") == 0) {
            ++count;
        }
    }
    return count;
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

bool opcodeAt(const std::string& mnemonic, const char* op) {
    const std::string name { op };
    if (mnemonic.compare(0, name.size(), name) != 0) {
        return false;
    }
    if (mnemonic.size() == name.size()) {
        return true;
    }
    const char next = mnemonic[name.size()];
    return next == ' ' || next == 'b' || next == 'w' || next == 'l' || next == 'q';
}

int countOpcode(const std::string& body, const char* op) {
    int count = 0;
    std::istringstream lines { body };
    std::string line;
    while (std::getline(lines, line)) {
        std::size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])) != 0) {
            ++i;
        }
        if (opcodeAt(line.substr(i), op)) {
            ++count;
        }
    }
    return count;
}

void expectOpcode(const char* source, const char* output, const char* op, int atOpt0, int atOpt) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    const int got = countOpcode(functionAssembly(program.readAssembly(), "f"), op);
    EXPECT_EQ(got, functionalTestOptFlag() == "-O0" ? atOpt0 : atOpt);
}

int countImuls(const std::string& body) {
    int count = 0;
    std::istringstream lines { body };
    std::string line;
    while (std::getline(lines, line)) {
        std::size_t i = 0;
        while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i])) != 0) {
            ++i;
        }
        if (line.compare(i, 4, "imul") == 0) {
            ++count;
        }
    }
    return count;
}

void expectImuls(const char* source, const char* output, int atOpt0, int atOpt) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    const int imuls = countImuls(functionAssembly(program.readAssembly(), "f"));
    EXPECT_EQ(imuls, functionalTestOptFlag() == "-O0" ? atOpt0 : atOpt);
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

TEST(Compiler, reusesAddAcrossEmptyIf) {
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
    )prg", "10", 3, 2);
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

TEST(Compiler, equalAddsAcrossFallthroughAreTaken) {
    expectCmps(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            x = a + b;
            goto L;
            L:
            return (a + b) == x;
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "1", 1, 0);
}

TEST(Compiler, changedAddAcrossFallthroughIsNotTaken) {
    expectCmps(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            x = a + b;
            a = a + 1;
            goto L;
            L:
            return (a + b) == x;
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "0", 1, 1);
}

TEST(Compiler, volatileEqualKeepsCompare) {
    expectCmps(R"prg(int printf(const char *, ...);
        int f(volatile int a, int b) {
            int x;
            x = a + b;
            goto L;
            L:
            return (a + b) == x;
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "1", 1, 1);
}

void expectAddSub(const char* source, const char* output, int adds0, int addsOpt, int subs0,
        int subsOpt) {
    SourceProgram program { source, { "-save-temps" } };
    program.compile();
    program.runAndExpect(output);
    const std::string body = functionAssembly(program.readAssembly(), "f");
    const bool opt0 = functionalTestOptFlag() == "-O0";
    EXPECT_EQ(countAdds(body), opt0 ? adds0 : addsOpt);
    EXPECT_EQ(countSubs(body), opt0 ? subs0 : subsOpt);
}

TEST(Compiler, combinesConstantAdds) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 4) + 3;
        }
        int main(void) {
            printf("%d", f(1));
            return 0;
        }
    )prg", "8", 2, 1, 0, 0);
}

TEST(Compiler, combinesConstantSubs) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n - 4) - 3;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "3", 0, 0, 2, 1);
}

TEST(Compiler, combinesOppositeAddThenSub) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 4) - 3;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "11", 1, 1, 1, 0);
}

TEST(Compiler, combinesOppositeSubThenAdd) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n - 4) + 3;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "9", 1, 0, 1, 1);
}

TEST(Compiler, combinesOuterAddOfInnerSub) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return 3 + (n - 4);
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "9", 1, 0, 1, 1);
}

TEST(Compiler, cancelsAddThenSub) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 4) - 4;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "10", 1, 0, 1, 0);
}

TEST(Compiler, sharedConstantCancelKeepsLaterUse) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            int k;
            k = 4;
            return (n + k) - k + k;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("14");
}

TEST(Compiler, cancelsSubThenAdd) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n - 4) + 4;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "10", 1, 0, 1, 0);
}

TEST(Compiler, combinesChainedOppositeThenAdd) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return ((n + 4) - 3) + 1;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "12", 2, 1, 1, 0);
}

TEST(Compiler, keepsOverflowingOppositeAddSub) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 2000000000) - (-2000000000);
        }
        int main(void) {
            printf("%d", 1);
            return 0;
        }
    )prg", "1", 1, 1, 1, 1);
}

TEST(Compiler, copiedConstantIsNotRewritten) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            volatile int v;
            int x;
            int c;
            int d;
            v = n;
            x = v;
            c = 4;
            d = c;
            return (x + 3) + d;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("17");
}

TEST(Compiler, reassignedConstantIsNotRewritten) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            volatile int v;
            int x;
            int c;
            v = n;
            x = v;
            c = 1;
            c = 3;
            return (x + 4) + c;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("17");
}

TEST(Compiler, reassignedConstantSubIsNotRewritten) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            volatile int v;
            int x;
            int c;
            v = n;
            x = v;
            c = 1;
            c = 3;
            return (x - 4) - c;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, storeBetweenConstantAddsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 4) + (n += 10, 3);
        }
        int main(void) {
            volatile int n;
            n = 1;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, storeBetweenConstantSubsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n - 4) - (n += 10, 3);
        }
        int main(void) {
            volatile int n;
            n = 10;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, callBetweenConstantAddsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int nglob;
        void bump(void) {
            int bytes;
            bytes = 1;
            char buf[bytes];
            buf[0] = 0;
            nglob = 100 + buf[0];
        }
        int f(void) {
            nglob = 1;
            return (nglob + 4) + (bump(), 3);
        }
        int main(void) {
            printf("%d", f());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, pointerStoreBetweenAddsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            int *p;
            p = &n;
            return (n + 4) + (*p = 10, 3);
        }
        int main(void) {
            volatile int n;
            n = 1;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, pointerStoreBetweenAddAndSubIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            int *p;
            p = &n;
            return (n + 4) - (*p = 1, 3);
        }
        int main(void) {
            volatile int n;
            n = 10;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("11");
}

TEST(Compiler, pointerStoreBetweenSubsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            int *p;
            p = &n;
            return (n - 4) - (*p = 1, 3);
        }
        int main(void) {
            volatile int n;
            n = 10;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, pointerStoreBetweenGlobalAddsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int g;
        int f(void) {
            int *p;
            g = 1;
            p = &g;
            return (g + 4) + (*p = 10, 3);
        }
        int main(void) {
            printf("%d", f());
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, vaStartBetweenConstantAddsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int g;
        int f(int n, ...) {
            g = 1;
            return (g + 4) + (__builtin_va_start(*(__builtin_va_list *)&g, n), 3);
        }
        int main(void) {
            volatile int n;
            n = 1;
            printf("%d", f(n, 9));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("8");
}

TEST(Compiler, reusesMultiplyBySameConstant) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n * 2) + (n * 2);
        }
        int main(void) {
            printf("%d", f(3));
            return 0;
        }
    )prg", "12", 2, 1);
}

TEST(Compiler, reusesMultiplyByHexConstant) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n * 2) + (n * 0x2);
        }
        int main(void) {
            printf("%d", f(3));
            return 0;
        }
    )prg", "12", 2, 1);
}

TEST(Compiler, reusesMultiplyByNegativeConstant) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n * -2) + (n * -2);
        }
        int main(void) {
            printf("%d", f(3));
            return 0;
        }
    )prg", "-12", 2, 1);
}

TEST(Compiler, keepsDistinctConstantMultiplies) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n * 2) + (n * 3);
        }
        int main(void) {
            printf("%d", f(3));
            return 0;
        }
    )prg", "15", 2, 2);
}

TEST(Compiler, keepsMultiplyWhenFactorChanges) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(int n, int k) {
            int a;
            a = n * 2;
            n = k;
            return a + (n * 2);
        }
        int main(void) {
            printf("%d", f(3, 7));
            return 0;
        }
    )prg", "20", 2, 2);
}

TEST(Compiler, volatileFactorIsMultipliedTwice) {
    expectImuls(R"prg(int printf(const char *, ...);
        int f(volatile int n) {
            return (n * 2) + (n * 2);
        }
        int main(void) {
            volatile int n;
            n = 3;
            printf("%d", f(n));
            return 0;
        }
    )prg", "12", 2, 2);
}

TEST(Compiler, combinesRepeatedConstantAdds) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 4) + 4;
        }
        int main(void) {
            printf("%d", f(1));
            return 0;
        }
    )prg", "9", 2, 1, 0, 0);
}

TEST(Compiler, keepsOverflowingConstantAdds) {
    expectAddSub(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n + 2000000000) + 2000000000;
        }
        int main(void) {
            printf("%d", 1);
            return 0;
        }
    )prg", "1", 2, 2, 0, 0);
}

TEST(Compiler, combinesConstantAnds) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n & 6) & 3;
        }
        int main(void) {
            printf("%d", f(7));
            return 0;
        }
    )prg", "2", "and", 2, 1);
}

TEST(Compiler, combinesLeftConstantAnd) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (3 & n) & 6;
        }
        int main(void) {
            printf("%d", f(7));
            return 0;
        }
    )prg", "2", "and", 2, 1);
}

TEST(Compiler, combinesChainedAnds) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return ((n & 7) & 3) & 1;
        }
        int main(void) {
            printf("%d", f(7));
            return 0;
        }
    )prg", "1", "and", 3, 1);
}

TEST(Compiler, combinesConstantOrs) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n | 1) | 2;
        }
        int main(void) {
            printf("%d", f(4));
            return 0;
        }
    )prg", "7", "or", 2, 1);
}

TEST(Compiler, orFillsAllBits) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n | 1) | ~1;
        }
        int main(void) {
            printf("%d", f(4));
            return 0;
        }
    )prg", "-1", "or", 2, 0);
}

TEST(Compiler, combinesConstantXors) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n ^ 1) ^ 2;
        }
        int main(void) {
            printf("%d", f(0));
            return 0;
        }
    )prg", "3", "xor", 2, 1);
}

TEST(Compiler, cancelsXor) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n ^ 4) ^ 4;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "10", "xor", 2, 0);
}

TEST(Compiler, sharedXorCancelKeepsLaterUse) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            int k;
            k = 4;
            return (n ^ k) ^ k ^ k;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "14", "xor", 3, 1);
}

TEST(Compiler, sharedAndMaskIsNotRewritten) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            int k;
            k = 3;
            return ((n & 6) & k) + k;
        }
        int main(void) {
            printf("%d", f(10));
            return 0;
        }
    )prg", "5", "and", 2, 2);
}

TEST(Compiler, combinesConstantLeftShifts) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n << 4) << 3;
        }
        int main(void) {
            printf("%d", f(1));
            return 0;
        }
    )prg", "128", "shl", 2, 1);
}

TEST(Compiler, keepsLeftShiftPastWidth) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(unsigned n) {
            return (n << 20) << 20;
        }
        int main(void) {
            printf("%u", f(1));
            return 0;
        }
    )prg", "0", "shl", 2, 2);
}

TEST(Compiler, combinesLongLeftShift) {
    expectOpcode(R"prg(int printf(const char *, ...);
        unsigned long long f(unsigned long long n) {
            return (n << 40) << 20;
        }
        int main(void) {
            printf("%llu", f(1));
            return 0;
        }
    )prg", "1152921504606846976", "shl", 2, 1);
}

TEST(Compiler, keepsLongShiftPastWidth) {
    expectOpcode(R"prg(int printf(const char *, ...);
        unsigned long long f(unsigned long long n) {
            return (n << 40) << 30;
        }
        int main(void) {
            printf("%llu", f(1));
            return 0;
        }
    )prg", "0", "shl", 2, 2);
}

TEST(Compiler, combinesArithmeticRightShifts) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int n) {
            return (n >> 1) >> 1;
        }
        int main(void) {
            printf("%d", f(-16));
            return 0;
        }
    )prg", "-4", "sar", 2, 1);
}

TEST(Compiler, combinesLogicalRightShifts) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(unsigned n) {
            return (n >> 1) >> 1;
        }
        int main(void) {
            printf("%u", f(4294967295u));
            return 0;
        }
    )prg", "1073741823", "shr", 2, 1);
}

TEST(Compiler, pointerStoreBetweenAndsIsNotReread) {
    SourceProgram program { R"prg(int printf(const char *, ...);
        int f(int n) {
            int *p;
            p = &n;
            return (n & 7) & (*p = 1, 3);
        }
        int main(void) {
            volatile int n;
            n = 7;
            printf("%d", f(n));
            return 0;
        }
    )prg" };
    program.compile();
    program.runAndExpect("3");
}

TEST(Compiler, reusesShiftAcrossEmptyIf) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            x = a << b;
            if (a) {}
            return x + (a << b);
        }
        int main(void) {
            printf("%d", f(3, 3));
            return 0;
        }
    )prg", "48", "shl", 2, 1);
}

TEST(Compiler, reusesShiftWhenOtherArmUsesDifferentDistance) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int a, int b, int d, int c) {
            int x;
            x = a << b;
            if (c) {
                x = a << d;
            }
            return x + (a << b);
        }
        int main(void) {
            printf("%d %d", f(3, 3, 1, 0), f(3, 3, 1, 1));
            return 0;
        }
    )prg", "48 30", "shl", 3, 2);
}

TEST(Compiler, keepsLiteralShiftAcrossEmptyIf) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(int a) {
            int x;
            x = a << 3;
            if (a) {}
            return x + (a << 3);
        }
        int main(void) {
            printf("%d", f(3));
            return 0;
        }
    )prg", "48", "shl", 2, 2);
}

TEST(Compiler, reusesAddAcrossEmptyIfAfterCall) {
    expectAdds(R"prg(int printf(const char *, ...);
        int g(int n) {
            int m;
            char buf[n];
            buf[0] = 0;
            m = buf[0];
            return m;
        }
        int f(int a, int b) {
            int x;
            g(1);
            x = a + b;
            if (a) {}
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 3, 2);
}

TEST(Compiler, keepsAddWhenOneArmChangesOperand) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b, int c) {
            int x;
            x = a + b;
            if (c) {
                a = a + 1;
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d %d", f(2, 3, 0), f(2, 3, 1));
            return 0;
        }
    )prg", "10 11", 4, 4);
}

TEST(Compiler, keepsAddComputedOnOneArm) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b, int c) {
            int x;
            x = 0;
            if (c) {
                x = a + b;
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d %d", f(2, 3, 0), f(2, 3, 1));
            return 0;
        }
    )prg", "5 10", 3, 3);
}

TEST(Compiler, keepsLaterAddWhenArmsDisagree) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b, int c) {
            int x;
            if (c) {
                x = a + b;
            } else {
                x = a + b;
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d %d", f(2, 3, 0), f(2, 3, 1));
            return 0;
        }
    )prg", "10 10", 4, 4);
}

TEST(Compiler, keepsAddWhenOneArmCalls) {
    expectAdds(R"prg(int printf(const char *, ...);
        int g(int n) {
            int m;
            char buf[n];
            buf[0] = 0;
            m = buf[0];
            return m;
        }
        int f(int a, int b, int c) {
            int x;
            x = a + b;
            if (c) {
                g(1);
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d %d", f(2, 3, 0), f(2, 3, 1));
            return 0;
        }
    )prg", "10 10", 3, 3);
}

TEST(Compiler, keepsVolatileAddAcrossEmptyIf) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(volatile int a, int b) {
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

TEST(Compiler, keepsAddWhenArmTakesAddress) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            int *p;
            x = a + b;
            if (a) {
                p = &a;
                *p = 1;
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "9", 3, 3);
}

TEST(Compiler, keepsAddAcrossLoop) {
    expectAdds(R"prg(int printf(const char *, ...);
        int f(int a, int b) {
            int x;
            int i;
            x = a + b;
            i = 0;
            while (i < a) {
                i = i + 1;
            }
            return x + (a + b);
        }
        int main(void) {
            printf("%d", f(2, 3));
            return 0;
        }
    )prg", "10", 4, 4);
}

TEST(Compiler, volatileAndIsNotCombined) {
    expectOpcode(R"prg(int printf(const char *, ...);
        int f(volatile int n) {
            return (n & 7) & 3;
        }
        int main(void) {
            volatile int n;
            n = 7;
            printf("%d", f(n));
            return 0;
        }
    )prg", "3", "and", 2, 2);
}

} // namespace
