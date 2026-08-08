#ifndef TEST_FIXTURES_H_
#define TEST_FIXTURES_H_

#include "driver/Compiler.h"
#include "driver/Configuration.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

using namespace testing;

AssemblyDialect functionalTestDialect();
std::string functionalTestDialectTag();

class Program {
  public:
    Program(std::string programName);
    virtual ~Program();

    void compile(bool verbose = false);

    void run();
    void run(std::string input);
    void runAndExpect(std::string expectedOutput);
    void runAndExpect(std::string input, std::string expectedOutput);
    void assertOutputEquals(std::string expectedOutput) const;
    void assertCompilationErrors(std::string expectedErrorFragment) const;

    std::string getOutputFilePath() const;
    std::string getName() const;
    std::string getSourceFilePath() const;
    std::string getExecutableFilePath() const;

  private:
    void assertCompiled() const;
    void assertExecuted() const;
    int compileOnce(bool verbose);
    static std::string executablePathFor(const std::string& sourcePath);
    static std::string outputPathFor(const std::string& sourcePath);

    const std::string programName;
    const std::string sourceFilePath;
    std::string compilationErrors;
    bool compiled = false;
    bool executed = false;
};

class SourceProgram : public Program {
  public:
    explicit SourceProgram(std::string sourceCode);

  private:
    const std::string programDirectory;
};

#endif /* TEST_FIXTURES_H_ */
