#ifndef TEST_FIXTURES_H_
#define TEST_FIXTURES_H_

#include "driver/Compiler.h"
#include "driver/Configuration.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

using namespace testing;

void compileFile(std::string sourceFilePath);

class Program {
  public:
    Program(std::string programName);
    virtual ~Program() = default;

    void compile();
    void run();
    void run(std::string input);
    void runAndExpect(std::string expectedOutput);
    void runAndExpect(std::string input, std::string expectedOutput);
    void assertOutputEquals(std::string expectedOutput) const;

    std::string getOutputFilePath() const;
    std::string getName() const;
    std::string getSourceFilePath() const;

  private:
    void assertCompiled() const;
    void assertExecuted() const;

    const std::string programName;
    const std::string sourceFilePath;
    const std::string executableFile;
    const std::string outputFile;
    bool compiled = false;
    bool executed = false;
};

class SourceProgram : public Program {
  public:
    SourceProgram(std::string sourceCode);
    SourceProgram(std::string sourceCode, std::string programName);

  private:
    const std::string programDirectory;
};

class CompilerConfiguration : public Configuration {
  public:
    CompilerConfiguration() {}
    virtual ~CompilerConfiguration() = default;

    std::vector<std::string> getSourceFileNames() const override { return {}; }
    std::string getGrammarFileName() const override;
    std::string getParsingTableFileName() const override;
    std::string getLexFileName() const override;
    bool usingCustomGrammar() const override { return false; }
    bool isParserLoggingEnabled() const override { return false; }
    bool isScannerLoggingEnabled() const override { return false; }
    bool isOutputIntermediateForms() const override { return true; }
};

#endif /* TEST_FIXTURES_H_ */
