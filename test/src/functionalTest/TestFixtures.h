#ifndef TEST_FIXTURES_H_
#define TEST_FIXTURES_H_

#include "driver/Compiler.h"
#include "driver/Configuration.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <vector>

using namespace testing;

std::string functionalTestDialectTag();
std::string functionalTestOptFlag();
std::string functionalTestMatrixTag();
std::vector<std::string> functionalTestFlags(std::vector<std::string> extra = {});

class Program {
  public:
    Program(std::string programName);
    virtual ~Program() = default;

    void addCompilerArg(std::string arg);
    void compile(bool verbose = false);

    void run();
    void run(std::string input);
    void runAndExpect(std::string expectedOutput);
    void runAndExpect(std::string input, std::string expectedOutput);
    void assertOutputEquals(std::string expectedOutput) const;
    void assertCompilationErrors(std::string expectedErrorFragment) const;
    std::string getCompilationErrors() const;

    std::string getOutputFilePath() const;
    std::string getSourceFilePath() const;
    std::string getExecutableFilePath() const;
    std::string getAssemblyFilePath() const;
    std::string readAssembly() const;

  private:
    void assertCompiled() const;
    void assertExecuted() const;
    int compileOnce(bool verbose);
    static std::string executablePathFor(const std::string& sourcePath);
    static std::string outputPathFor(const std::string& sourcePath);

    const std::string programName;
    const std::string sourceFilePath;
    std::vector<std::string> extraCompilerArgs;
    std::string compilationErrors;
    bool compiled = false;
    bool executed = false;
};

class SourceProgram : public Program {
  public:
    explicit SourceProgram(std::string sourceCode, std::vector<std::string> extraCompilerArgs = {});

  private:
    const std::string programDirectory;
};

#endif /* TEST_FIXTURES_H_ */
