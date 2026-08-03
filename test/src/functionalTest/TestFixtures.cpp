#include "TestFixtures.h"

#include "driver/Compiler.h"
#include "driver/CompilerComponentsFactory.h"
#include "driver/Driver.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "translation_unit/TranslationUnit.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <atomic>
#include <cstdlib>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>

#include "ResourceHelpers.h"
#include "util/Logger.h"
#include "util/LogManager.h"
#include "util/Process.h"
#include "util/SourcePath.h"

using namespace testing;

namespace {

// Default dialect for a bare `./functionalTest` run is Intel. Product CI must
// exercise AT&T too: ctest registers functionalTest_att_shard* with
// TRANS_ASM_DIALECT=att (see test/src/CMakeLists.txt). Do not treat a green
// default functional binary as dual-backend coverage by itself.
AssemblyDialect dialectFromEnvironment() {
    const char* raw = std::getenv("TRANS_ASM_DIALECT");
    if (raw == nullptr || raw[0] == '\0' || std::string(raw) == "intel") {
        return AssemblyDialect::Intel;
    }
    if (std::string(raw) == "att") {
        return AssemblyDialect::AtAndT;
    }
    throw std::runtime_error {
            std::string("TRANS_ASM_DIALECT must be 'intel' or 'att' (got '") + raw + "')" };
}

const AssemblyDialect kFunctionalTestDialect = dialectFromEnvironment();

} // namespace

std::string functionalTestDialectTag() {
    return assemblyDialectTag(kFunctionalTestDialect);
}


namespace {

// Prefer the current gtest name so parallel/serial runs do not clobber a shared
// programs/tmp/test.c; fall back to a process-local counter outside a test.
std::string defaultSourceProgramName() {
    // Dialect suffix is applied in SourceProgram ctor (intel/att shards share no paths).
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

Program::Program(std::string programName) :
    programName{programName},
    sourceFilePath{getTestResourcePath("programs/" + programName + ".c")} ,
    executableFile{sourceFilePath + ".out"},
    outputFile{sourceFilePath + ".execution.output"} {

    remove(executableFile.c_str());
    remove(outputFile.c_str());
}

void Program::addCompilerArg(std::string arg) {
    extraCompilerArgs.push_back(std::move(arg));
}

int Program::compileOnce(bool verbose) {
    std::vector<std::string> arguments{"trans", "--resources=../../../"};
    arguments.push_back("-masm=" + functionalTestDialectTag());
    arguments.insert(arguments.end(), extraCompilerArgs.begin(), extraCompilerArgs.end());
    arguments.push_back(sourceFilePath);
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    std::stringstream outputStream;
    std::stringstream errorStream;
    int exitCode = 0;

    LogManager::withOutputStreams(outputStream, errorStream, [&argv, &exitCode]() {
        Driver transDriver{};
        exitCode = transDriver.run((int)argv.size() - 1, argv.data());
    });
    if (verbose) {
        std::cout << outputStream.str();
    }

    compilationErrors = errorStream.str();
    return exitCode;
}

void Program::compile(bool verbose) {
    compiled = (compileOnce(verbose) == 0);
    if (!compiled) {
        if (!compilationErrors.empty()) {
            std::cerr << compilationErrors;
        }
        if (compilationErrors.empty()) {
            compilationErrors = "driver returned status 1";
        }
    }
}

void Program::compileWithoutPreprocess(bool verbose) {
    compileWithArgs({ "--no-preprocess", sourceFilePath }, verbose);
}

void Program::compileWithArgs(const std::vector<std::string>& extraArgs, bool verbose) {
    std::vector<std::string> arguments {"trans", "--resources=../../../"};
    arguments.push_back("-masm=" + functionalTestDialectTag());
    for (const auto& arg : extraCompilerArgs) {
        arguments.push_back(arg);
    }
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

    int exitCode = 0;
    LogManager::withOutputStreams(outputStream, errorStream, [&argv, &exitCode](){
        Driver transDriver {};
        exitCode = transDriver.run((int)argv.size()-1, argv.data());
    });
    if (verbose) {
        std::cout << outputStream.str();
    }

    compilationErrors = errorStream.str();
    if (exitCode == 0) {
        compiled = true;
    } else {
        if (!compilationErrors.empty()) {
            std::cerr << compilationErrors;
        }
        if (compilationErrors.empty()) {
            compilationErrors = "driver returned status " + std::to_string(exitCode);
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

std::string Program::getCompilationErrors() const {
    return compilationErrors;
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

std::string Program::getAssemblyFilePath() const {
    return util::withExtension(sourceFilePath, ".s");
}

std::string Program::readAssembly() const {
    assertCompiled();
    const std::string path = getAssemblyFilePath();
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error { "assembly not found (compile with -save-temps): " + path };
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
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
SourceProgram::SourceProgram(std::string sourceCode, std::vector<std::string> extraArgs) :
    SourceProgram(std::move(sourceCode), std::move(extraArgs), defaultSourceProgramName()) {}

SourceProgram::SourceProgram(std::string sourceCode, std::string programName) :
    SourceProgram(std::move(sourceCode), {}, std::move(programName)) {}

SourceProgram::SourceProgram(std::string sourceCode, std::vector<std::string> extraArgs,
        std::string programName) :
    Program{"tmp/" + programName + "_" + functionalTestDialectTag()},
    programDirectory{getTestProgramsTmpDir()}
{
    for (auto& arg : extraArgs) {
        addCompilerArg(std::move(arg));
    }
    ensureTestProgramsTmpDir();

    std::ofstream programFile{getSourceFilePath()};
    if (!programFile) {
        throw std::runtime_error("Could not write temp source " + getSourceFilePath());
    }
    programFile << sourceCode;
}

