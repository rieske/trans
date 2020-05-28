#include "driver/Compiler.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/Configuration.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "translation_unit/TranslationUnit.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <sys/stat.h>

#include "ResourceHelpers.h"

using namespace testing;
using namespace parser;

namespace {

std::string readFileContents(std::string filename) {
    std::ifstream inputStream(filename);
    std::string content;

    inputStream.seekg(0, std::ios::end);
    content.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);

    content.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
    return content;
}

void callSystem(std::string command) { system(command.c_str()); }

class CompilerConfiguration : public Configuration {
  public:
    CompilerConfiguration() {}
    virtual ~CompilerConfiguration() {}

    std::vector<std::string> getSourceFileNames() const override { return {}; }
    std::string getGrammarFileName() const override { return getResourcePath("configuration/grammar.bnf"); }
    std::string getParsingTableFileName() const override { return getResourcePath("configuration/parsing_table"); }
    std::string getLexFileName() const override { return getResourcePath("configuration/scanner.lex"); }
    bool usingCustomGrammar() const override { return false; }
    bool isParserLoggingEnabled() const override { return false; }
    bool isScannerLoggingEnabled() const override { return false; }
};

class Program {
  public:
    Program(std::string programName) :
        programName{programName},
        sourceFilePath{getTestResourcePath("programs/" + programName + ".src")} ,
        executableFile{sourceFilePath + ".out"},
        outputFile{sourceFilePath + ".execution.output"} {

        remove(executableFile.c_str());
        remove(outputFile.c_str());
    }

    virtual ~Program() = default;

    void compile() {
        CompilerConfiguration configuration;
        Compiler compiler{new CompilerComponentsFactory{configuration}};

        compiler.compile(sourceFilePath);
        compiled = true;
    }

    void run() {
        assertCompiled();
        remove(outputFile.c_str());
        callSystem(executableFile + " > " + outputFile);
        executed = true;
    }

    void run(std::string input) {
        assertCompiled();
        remove(outputFile.c_str());
        callSystem("echo '" + input + "' | " + executableFile + " > " + outputFile);
        executed = true;
    }

    void runAndExpect(std::string expectedOutput) {
        run();
        assertOutputEquals(expectedOutput);
    }

    void runAndExpect(std::string input, std::string expectedOutput) {
        run(input);
        assertOutputEquals(expectedOutput);
    }

    void assertOutputEquals(std::string expectedOutput) const {
        assertExecuted();
        EXPECT_THAT(readFileContents(outputFile), Eq(expectedOutput));
    }

    std::string getOutputFilePath() const {
        assertExecuted();
        return outputFile;
    }

    std::string getName() const {
        return programName;
    }

    std::string getSourceFilePath() const {
        return sourceFilePath;
    }


  private:
    void assertCompiled() const {
        if (!compiled) {
            throw std::runtime_error{"Program is not compiled."};
        }
    }

    void assertExecuted() const {
        if (!executed) {
            throw std::runtime_error{"Program has not executed."};
        }
    }

    const std::string programName;
    const std::string sourceFilePath;
    const std::string executableFile;
    const std::string outputFile;
    bool compiled = false;
    bool executed = false;
};

class SourceProgram : public Program {
  public:
    SourceProgram(std::string sourceCode) : SourceProgram(sourceCode, "test") {}
    SourceProgram(std::string sourceCode, std::string programName) : Program{"tmp/" + programName}, programDirectory{getTestResourcePath("programs/tmp/")} {
        if (mkdir(programDirectory.c_str(), 0777) == -1 && errno != 17) {
            throw std::runtime_error("Could not create directory " + programDirectory + ": " + std::to_string(errno) + ":" + strerror(errno));
        }

        std::ofstream programFile{getSourceFilePath()};
        programFile << sourceCode;
        programFile.close();
    }

  private:
    const std::string programDirectory;
};

TEST(Compiler, throwsForNonExistentFile) {
    CompilerConfiguration configuration;
    Compiler compiler{new CompilerComponentsFactory{configuration}};

    ASSERT_THROW(compiler.compile("nonexistentSourceFileName"), std::runtime_error);
}

TEST(Compiler, compilesFibonacciProgram) {
    SourceProgram program{R"prg(
        int fib (int n1, int n2, int max)
        {
            int fibb = n1 + n2;
            output n2;
            if (fibb > max)
                return fibb;
            else
                return fib(n2, fibb, max);
        }

        int main()
        {
            int a;
            int b;
            int max;
            input max;
            a = 1;
            b = 1;
            output fib(a, b, max);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42", "1\n2\n3\n5\n8\n13\n21\n34\n55\n");
}

TEST(Compiler, compilesSwapProgram) {
    SourceProgram program{R"prg(
        int swap(int *x, int *y)
        {
            int temp;
            output *x;
            output *y;
            temp = *x;
            *x = *y;
            *y = temp;
            output *x;
            output *y;
            return 0;
        }

        int main()
        {
            int a = 0;
            int b = 1;
            output a;
            output b;
            output &a;
            output &b;
            swap (&a, &b);
            output a;
            output b;
            output &a;
            output &b;
            return 0;
        }
    )prg"};
    program.compile();
    program.run();

    std::ifstream expectedOutputStream{program.getOutputFilePath()};
    std::string outputLine;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    std::string firstAddressBefore;
    expectedOutputStream >> firstAddressBefore;
    std::string secondAddressBefore;
    expectedOutputStream >> secondAddressBefore;
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("1"));
    expectedOutputStream >> outputLine;
    EXPECT_THAT(outputLine, Eq("0"));
    std::string firstAddressAfter;
    expectedOutputStream >> firstAddressAfter;
    std::string secondAddressAfter;
    expectedOutputStream >> secondAddressAfter;
    EXPECT_THAT(firstAddressBefore, Not(Eq(secondAddressBefore)));
    EXPECT_THAT(firstAddressBefore, Eq(firstAddressAfter));
    EXPECT_THAT(secondAddressBefore, Eq(secondAddressAfter));
}

TEST(Compiler, compilesSimpleOutputProgram) {
    SourceProgram program{R"prg(
        int main()
        {
            int a = 1;
            output a;
            output -a;
            output -(-a);
            output 2-5;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1\n-1\n1\n-3\n");
}

TEST(Compiler, inputOutput) {
    SourceProgram program{R"prg(
        int main() {
            int a;
            input a;
            output a;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("42", "42\n");
    program.runAndExpect("-42", "-42\n");
}

TEST(Compiler, compilesWhileLoopFactorialProgram) {
    SourceProgram program{R"prg(
        int factorialWhile(int n) {
            int result = 1;
            while(n > 1) {
                result = result * n;
                n = n - 1;
            }
            return result;
        }

        int main() {
            int a;
            input a;
            output factorialWhile(a);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("5", "120\n");
    program.runAndExpect("1", "1\n");
    program.runAndExpect("0", "1\n");
    // FIXME
    // program.runAndExpect("-1", "-1\n");
    // program.runAndExpect("-5", "-120\n");
}

TEST(Compiler, compilesWhileLoopSumProgram) {
    SourceProgram program{R"prg(
        int whileSum(int it, int add) {
            int result = 0;
            while(it > 0) {
                result = result + add;
                it = it - 1;
            }
            return result;
        }

        int main() {
            output whileSum(5, 2);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10\n");
}

TEST(Compiler, compilesForLoopFactorialProgram) {
    SourceProgram program{R"prg(
        int factorialFor(int n) {
            int i;
            int result = 1;
            for (i = 1; i <= n; i++) {
                result *= i;
            }
            return result;
        }

        int main() {
            int n;
            input n;
            output factorialFor(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("5", "120\n");
    program.runAndExpect("1", "1\n");
    program.runAndExpect("0", "1\n");
    // FIXME
    //program.runAndExpect("-1", "-1\n");
    // program.runAndExpect("-5", "-120\n");
}


TEST(Compiler, compilesForLoopSumProgram) {
    SourceProgram program{R"prg(
        int forSum(int it, int add) {
            int i;
            int result = 0;
            for (i = 0; i < it; i++) {
                result = result + add;
            }
            return result;
        }

        int main() {
            output forSum(5, 2);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10\n");
}

TEST(Compiler, forLoopIterationOutput) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i <= n; i++) {
                output i;
            }
            return 0;
        }

        int main() {
            int n;
            input n;
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n");
    program.runAndExpect("5", "0\n1\n2\n3\n4\n5\n");
    program.runAndExpect("1", "0\n1\n");
    program.runAndExpect("0", "0\n");
    // FIXME
    // program.runAndExpect("-1", "");
}

TEST(Compiler, forLoopLessThan) {
    SourceProgram program{R"prg(
        int iterationOutput(int n) {
            int i;
            for (i = 0; i < n; i++) {
                output i;
            }
            return 0;
        }

        int main() {
            int n;
            input n;
            iterationOutput(n);
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("10", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9\n");
    program.runAndExpect("5", "0\n1\n2\n3\n4\n");
    program.runAndExpect("1", "0\n");
    program.runAndExpect("0", "");
    // FIXME
    // program.runAndExpect("-1", "");
}

TEST(Compiler, increments) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            input n;
            output n++;
            output n;
            output ++n;
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "1\n2\n3\n3\n");
    program.runAndExpect("0", "0\n1\n2\n2\n");
    program.runAndExpect("-1", "-1\n0\n1\n1\n");
    program.runAndExpect("-3", "-3\n-2\n-1\n-1\n");
}

TEST(Compiler, decrements) {
    SourceProgram program{R"prg(
        int main() {
            int n;
            input n;
            output n--;
            output n;
            output --n;
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "3\n2\n1\n1\n");
    program.runAndExpect("2", "2\n1\n0\n0\n");
    program.runAndExpect("1", "1\n0\n-1\n-1\n");
    program.runAndExpect("-1", "-1\n-2\n-3\n-3\n");
}

TEST(Compiler, incrementsFunctions) {
    SourceProgram program{R"prg(
        int pre(int i) {
            return ++i;
        }

        int post(int i) {
            return i++;
        }

        int main() {
            int n;
            input n;
            output pre(n);
            output post(n);
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "2\n1\n1\n");
    program.runAndExpect("0", "1\n0\n0\n");
    program.runAndExpect("-1", "0\n-1\n-1\n");
    program.runAndExpect("-3", "-2\n-3\n-3\n");
}

TEST(Compiler, decrementsFunctions) {
    SourceProgram program{R"prg(
        int pre(int i) {
            return --i;
        }

        int post(int i) {
            return i--;
        }

        int main() {
            int n;
            input n;
            output pre(n);
            output post(n);
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("3", "2\n3\n3\n");
    program.runAndExpect("2", "1\n2\n2\n");
    program.runAndExpect("1", "0\n1\n1\n");
    program.runAndExpect("-1", "-2\n-1\n-1\n");
}

// FIXME
/*
TEST(Compiler, incrementsFunctionsPointers) {
    SourceProgram program{R"prg(
        int pre(int* i) {
            ++(*i);
            // FIXME void returns
            return 0;
        }

        int post(int* i) {
            (*i)++;
            return 0;
        }

        int main() {
            int n;
            input n;
            pre(&n);
            output n;
            post(&n);
            output n;
            return 0;
        }
    )prg"};

    program.compile();

    program.runAndExpect("1", "2\n3\n");
    program.runAndExpect("0", "1\n2\n");
    program.runAndExpect("-1", "0\n1\n");
    program.runAndExpect("-3", "-2\n-1\n");
}
*/

TEST(Compiler, voidReturnExplicit) {
    SourceProgram program{R"prg(
        void voidRet() {
            return;
        }

        int main() {
            voidRet();
            return 0;
        }
    )prg"};

    program.compile();

    program.run();
}

/*TEST(Compiler, voidReturnImplicit) {
    SourceProgram program{R"prg(
        void voidRet() {
        }

        int main() {
            voidRet();
            return 0;
        }
    )prg", "bar"};

    program.compile();

    program.run();
}*/

} // namespace
