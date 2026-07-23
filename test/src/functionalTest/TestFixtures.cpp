#include "TestFixtures.h"

#include "driver/Compiler.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/ConfigurationParser.h"
#include "driver/Driver.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "translation_unit/TranslationUnit.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <sys/stat.h>

#include "ResourceHelpers.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"

using namespace testing;

std::string readFileContents(std::string filename) {
    std::ifstream inputStream(filename);
    std::string content;

    inputStream.seekg(0, std::ios::end);
    content.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);

    content.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
    return content;
}

Program::Program(std::string programName) :
    programName{programName},
    sourceFilePath{getTestResourcePath("programs/" + programName + ".src")} ,
    executableFile{sourceFilePath + ".out"},
    outputFile{sourceFilePath + ".execution.output"} {

    remove(executableFile.c_str());
    remove(outputFile.c_str());
}

void Program::compile(bool verbose) {
    std::vector<std::string> arguments {"trans", "-r../../../"};
    arguments.push_back("-lti"); // log (l) syntax tree (t) and intermediate form (i)
    arguments.push_back(sourceFilePath);
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    std::stringstream outputStream;
    std::stringstream errorStream;
    int exitCode = 0;

    LogManager::withOutputStreams(outputStream, errorStream, [&argv, &exitCode](){
        Driver transDriver {};
        exitCode = transDriver.run(ConfigurationParser {(int)argv.size()-1, argv.data()});
    });
    if (verbose) {
        std::cout << outputStream.str();
    }

    compilationErrors = errorStream.str();
    // Driver already writes failures to the error logger; exit code is the contract.
    if (exitCode == 0) {
        compiled = true;
    } else {
        if (!compilationErrors.empty()) {
            std::cerr << compilationErrors;
        }
        compiled = false;
    }
}

void Program::run() {
    assertCompiled();
    remove(outputFile.c_str());
    util::runProcessOrThrow({ executableFile }, {}, outputFile);
    executed = true;
}

void Program::run(std::string input) {
    assertCompiled();
    remove(outputFile.c_str());
    // Match prior `echo '...' | prog` behavior: stdin text ends with a newline.
    util::runProcessOrThrow({ executableFile }, input + "\n", outputFile);
    executed = true;
}

void Program::runAndExpect(std::string expectedOutput) {
    run();
    assertOutputEquals(expectedOutput);
}

void Program::runAndExpect(std::string input, std::string expectedOutput) {
    run(input);
    assertOutputEquals(expectedOutput);
}

void Program::assertOutputEquals(std::string expectedOutput) const {
    assertExecuted();
    EXPECT_THAT(readFileContents(outputFile), Eq(expectedOutput));
}

void Program::assertCompilationErrors(std::string expectedErrorFragment) const {
    if (compiled) {
        throw std::runtime_error{"Program is compiled without errors."};
    }
    EXPECT_THAT(compilationErrors, HasSubstr(expectedErrorFragment));
}

std::string Program::getOutputFilePath() const {
    assertExecuted();
    return outputFile;
}

std::string Program::getName() const {
    return programName;
}

std::string Program::getSourceFilePath() const {
    return sourceFilePath;
}

void Program::assertCompiled() const {
    if (!compiled) {
        throw std::runtime_error{"Program is not compiled."};
    }
}

void Program::assertExecuted() const {
    if (!executed) {
        throw std::runtime_error{"Program has not executed."};
    }
}

namespace {

// Unique basename under programs/tmp/ so concurrent processes (ctest -j / gtest
// shards) do not clobber each other's .src / .S / .o / .out artifacts.
// Replace path separators: parameterized suites use names like
// "Suite/Case.Name/param" which must stay a single path segment.
std::string uniqueProgramNameForCurrentTest() {
    const auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
    if (info == nullptr) {
        throw std::logic_error(
                "SourceProgram requires an active gtest (current_test_info is null)");
    }
    std::string name = std::string(info->test_suite_name()) + "_" + info->name();
    for (char& c : name) {
        if (c == '/' || c == '\\') {
            c = '_';
        }
    }
    return name;
}

} // namespace

SourceProgram::SourceProgram(std::string sourceCode) :
    Program{"tmp/" + uniqueProgramNameForCurrentTest()},
    programDirectory{getTestResourcePath("programs/tmp/")}
{
    if (mkdir(programDirectory.c_str(), 0777) == -1 && errno != 17) {
        throw std::runtime_error("Could not create directory " + programDirectory + ": " + std::to_string(errno) + ":" + strerror(errno));
    }

    std::ofstream programFile{getSourceFilePath()};
    programFile << sourceCode;
    programFile.close();
}

