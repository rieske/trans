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

#include <atomic>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>

#include "ResourceHelpers.h"
#include "util/Logger.h"
#include "util/LogManager.h"

using namespace testing;

namespace {

// Prefer the current gtest name so parallel/serial runs do not clobber a shared
// programs/tmp/test.src; fall back to a process-local counter outside a test.
std::string defaultSourceProgramName() {
    const TestInfo* info = UnitTest::GetInstance()->current_test_info();
    if (info != nullptr) {
        std::string name = std::string(info->test_suite_name()) + "_" + info->name();
        for (char& c : name) {
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
                c = '_';
            }
        }
        return name;
    }
    static std::atomic<unsigned> counter { 0 };
    return "source_" + std::to_string(counter++);
}

} // namespace

std::string readFileContents(std::string filename) {
    std::ifstream inputStream(filename);
    std::string content;

    inputStream.seekg(0, std::ios::end);
    content.reserve(inputStream.tellg());
    inputStream.seekg(0, std::ios::beg);

    content.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
    return content;
}

void callSystem(std::string command) {
    int returnCode = system(command.c_str());
    if (returnCode != 0) {
        throw std::runtime_error { "Unexpected return code: " + std::to_string(returnCode) };
    }
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
    // Functional tests write self-contained sources; skip gcc -E so line numbers in
    // diagnostics match the source the test author wrote.
    arguments.push_back("--no-preprocess");
    arguments.push_back(sourceFilePath);
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    std::stringstream outputStream;
    std::stringstream errorStream;

    int status = 0;
    LogManager::withOutputStreams(outputStream, errorStream, [&argv, &status](){
        Driver transDriver {};
        status = transDriver.run(ConfigurationParser {(int)argv.size()-1, argv.data()});
    });
    if (verbose) {
        std::cout << outputStream.str();
    }

    compilationErrors = errorStream.str();
    if (compilationErrors.empty() && status == 0) {
        compiled = true;
    } else {
        if (!compilationErrors.empty()) {
            std::cerr << compilationErrors;
        }
        if (status != 0 && compilationErrors.empty()) {
            compilationErrors = "driver returned status " + std::to_string(status);
        }
        compiled = false;
    }
}

void Program::compileWithPreprocess(bool verbose) {
    compileWithArgs({ "-lti", sourceFilePath }, verbose);
}

void Program::compileWithArgs(const std::vector<std::string>& extraArgs, bool verbose) {
    std::vector<std::string> arguments {"trans", "-r../../../"};
    for (const auto& arg : extraArgs) {
        arguments.push_back(arg);
    }
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    std::stringstream outputStream;
    std::stringstream errorStream;

    int status = 0;
    LogManager::withOutputStreams(outputStream, errorStream, [&argv, &status](){
        Driver transDriver {};
        status = transDriver.run(ConfigurationParser {(int)argv.size()-1, argv.data()});
    });
    if (verbose) {
        std::cout << outputStream.str();
    }

    compilationErrors = errorStream.str();
    if (compilationErrors.empty() && status == 0) {
        compiled = true;
    } else {
        if (!compilationErrors.empty()) {
            std::cerr << compilationErrors;
        }
        if (status != 0 && compilationErrors.empty()) {
            compilationErrors = "driver returned status " + std::to_string(status);
        }
        compiled = false;
    }
}

void Program::run() {
    assertCompiled();
    remove(outputFile.c_str());
    callSystem(executableFile + " > " + outputFile);
    executed = true;
}

void Program::run(std::string input) {
    assertCompiled();
    remove(outputFile.c_str());
    callSystem("echo '" + input + "' | " + executableFile + " > " + outputFile);
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

std::string Program::getExecutablePath() const {
    return executableFile;
}

bool Program::isCompiled() const {
    return compiled;
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

// Parallel-safe: defaultSourceProgramName() uses gtest suite_name + test name
// (master ctest -j / shards) with a process-local fallback outside a test.
SourceProgram::SourceProgram(std::string sourceCode) :
    SourceProgram(sourceCode, defaultSourceProgramName()) {}

SourceProgram::SourceProgram(std::string sourceCode, std::string programName) :
    Program{"tmp/" + programName}, programDirectory{getTestProgramsTmpDir()}
{
    ensureTestProgramsTmpDir();

    std::ofstream programFile{getSourceFilePath()};
    if (!programFile) {
        throw std::runtime_error("Could not write temp source " + getSourceFilePath());
    }
    programFile << sourceCode;
}

