#include "TestFixtures.h"

#include "driver/Compiler.h"
#include "driver/Driver.h"
#include "scanner/Scanner.h"
#include "scanner/Token.h"
#include "semantic_analyzer/SemanticAnalyzer.h"
#include "translation_unit/TranslationUnit.h"
#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <sys/stat.h>

#include "ResourceHelpers.h"
#include "util/LogManager.h"
#include "util/Logger.h"
#include "util/Process.h"
#include "util/SourcePath.h"

using namespace testing;

std::string readFileContents(std::string filename) {
    std::ifstream inputStream(filename);
    if (!inputStream) {
        throw std::runtime_error { "unable to read file: " + filename };
    }
    std::string content;
    inputStream.seekg(0, std::ios::end);
    content.reserve(static_cast<std::size_t>(inputStream.tellg()));
    inputStream.seekg(0, std::ios::beg);
    content.assign(std::istreambuf_iterator<char>(inputStream), std::istreambuf_iterator<char>());
    return content;
}

namespace {

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

std::string Program::executablePathFor(const std::string& sourcePath) {
    return sourcePath + ".out";
}

std::string Program::outputPathFor(const std::string& sourcePath) {
    return sourcePath + ".execution.output";
}

Program::Program(std::string programName) :
        programName{programName},
        sourceFilePath{getTestResourcePath("programs/" + programName + ".c")} {
    remove(executablePathFor(sourceFilePath).c_str());
    remove(outputPathFor(sourceFilePath).c_str());
}

void Program::addCompilerArg(std::string arg) {
    extraCompilerArgs.push_back(std::move(arg));
}

int Program::compileOnce(bool verbose) {
    std::vector<std::string> arguments{"trans", "--resources=../../../"};
    arguments.push_back("-masm=" + functionalTestDialectTag());
    arguments.insert(arguments.end(), extraCompilerArgs.begin(), extraCompilerArgs.end());
    arguments.push_back(sourceFilePath);
    std::vector<char *> argv;
    for (const auto &arg : arguments) {
        argv.push_back((char *)arg.data());
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
        std::cout << "[backend=" << functionalTestDialectTag() << "]\n" << outputStream.str();
    }

    compilationErrors = errorStream.str();
    if (exitCode != 0 && !compilationErrors.empty()) {
        std::cerr << "[backend=" << functionalTestDialectTag() << "]\n" << compilationErrors;
    }
    return exitCode;
}

void Program::compile(bool verbose) {
    compilationErrors.clear();
    compiled = (compileOnce(verbose) == 0);
}

void Program::run() {
    assertCompiled();
    const std::string outFile = outputPathFor(sourceFilePath);
    remove(outFile.c_str());
    util::runProcessOrThrow({executablePathFor(sourceFilePath)}, {}, outFile);
    executed = true;
}

void Program::run(std::string input) {
    assertCompiled();
    const std::string outFile = outputPathFor(sourceFilePath);
    remove(outFile.c_str());
    // Match prior `echo '...' | prog` behavior: stdin text ends with a newline.
    util::runProcessOrThrow({executablePathFor(sourceFilePath)}, input + "\n", outFile);
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
    SCOPED_TRACE(std::string("backend=") + functionalTestDialectTag());
    EXPECT_THAT(readFileContents(outputPathFor(sourceFilePath)), Eq(expectedOutput));
}

void Program::assertCompilationErrors(std::string expectedErrorFragment) const {
    if (compiled) {
        throw std::runtime_error{"Program is compiled without errors."};
    }
    SCOPED_TRACE(std::string("backend=") + functionalTestDialectTag());
    EXPECT_THAT(compilationErrors, HasSubstr(expectedErrorFragment));
}

std::string Program::getCompilationErrors() const {
    return compilationErrors;
}

std::string Program::getOutputFilePath() const {
    assertExecuted();
    return outputPathFor(sourceFilePath);
}

std::string Program::getExecutableFilePath() const {
    return executablePathFor(sourceFilePath);
}

std::string Program::getSourceFilePath() const { return sourceFilePath; }

std::string Program::getAssemblyFilePath() const {
    return util::withExtension(sourceFilePath, ".s");
}

std::string Program::readAssembly() const {
    assertCompiled();
    const std::string path = getAssemblyFilePath();
    try {
        return readFileContents(path);
    } catch (const std::runtime_error&) {
        throw std::runtime_error { "assembly not found (compile with -save-temps): " + path };
    }
}

void Program::assertCompiled() const {
    if (!compiled) {
        throw std::runtime_error{
                std::string("Program is not compiled [backend=") + functionalTestDialectTag() + "]."};
    }
}

void Program::assertExecuted() const {
    if (!executed) {
        throw std::runtime_error{
                std::string("Program has not executed [backend=") + functionalTestDialectTag() + "]."};
    }
}

namespace {

std::string uniqueProgramNameForCurrentTest() {
    const auto *info = ::testing::UnitTest::GetInstance()->current_test_info();
    if (info == nullptr) {
        throw std::logic_error("SourceProgram requires an active gtest (current_test_info is null)");
    }
    std::string name = std::string(info->test_suite_name()) + "_" + info->name()
            + "_" + functionalTestDialectTag();
    for (char &c : name) {
        if (c == '/' || c == '\\') {
            c = '_';
        }
    }
    return name;
}

} // namespace

SourceProgram::SourceProgram(std::string sourceCode, std::vector<std::string> extraArgs) :
        Program{"tmp/" + uniqueProgramNameForCurrentTest()},
        programDirectory{getTestResourcePath("programs/tmp/")} {
    for (auto& arg : extraArgs) {
        addCompilerArg(std::move(arg));
    }
    if (mkdir(programDirectory.c_str(), 0777) == -1 && errno != 17) {
        throw std::runtime_error("Could not create directory " + programDirectory + ": " + std::to_string(errno) + ":" + strerror(errno));
    }

    std::ofstream programFile{getSourceFilePath()};
    programFile << sourceCode;
    programFile.close();
}
