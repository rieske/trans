#ifndef TEST_FIXTURES_H_
#define TEST_FIXTURES_H_

#include "driver/Compiler.h"
#include "driver/Configuration.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>
#include <vector>

using namespace testing;

class Program {
  public:
    Program(std::string programName);
    virtual ~Program() = default;

    void compile(bool verbose = false);
    // Like compile(), but runs the gcc -E preprocessor (for #include / #define tests).
    void compileWithPreprocess(bool verbose = false);
    // Invoke the driver with extra CLI flags after the usual -r base path.
    // Does not add --no-preprocess (caller must pass it when desired).
    void compileWithArgs(const std::vector<std::string>& extraArgs, bool verbose = false);
    void run();
    void run(std::string input);
    void runAndExpect(std::string expectedOutput);
    void runAndExpect(std::string input, std::string expectedOutput);
    void assertOutputEquals(std::string expectedOutput) const;
    void assertCompilationErrors(std::string expectedErrorFragment) const;

    std::string getOutputFilePath() const;
    std::string getName() const;
    std::string getSourceFilePath() const;
    // Executable path produced by a full compile+link (source + ".out").
    std::string getExecutablePath() const;
    bool isCompiled() const;

  private:
    void assertCompiled() const;
    void assertExecuted() const;

    const std::string programName;
    const std::string sourceFilePath;
    const std::string executableFile;
    const std::string outputFile;
    std::string compilationErrors;
    bool compiled = false;
    bool executed = false;
};

class SourceProgram : public Program {
  public:
    // Writes under programs/tmp/<Suite>_<Name>.* so parallel ctest/gtest shards
    // do not collide. Optional name for multi-source tests that need stable paths.
    explicit SourceProgram(std::string sourceCode);
    SourceProgram(std::string sourceCode, std::string programName);

  private:
    const std::string programDirectory;
};

#endif /* TEST_FIXTURES_H_ */
