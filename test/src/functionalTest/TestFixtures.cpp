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
    if (verbose) {
        arguments.push_back("-li");
    }
    arguments.push_back(sourceFilePath);
    std::vector<char*> argv;
    for (const auto& arg : arguments) {
        argv.push_back((char*)arg.data());
    }
    argv.push_back(nullptr);

    std::stringstream errorStream;

    LogManager::withErrorStream(errorStream, [&argv](){
        Driver transDriver {};
        transDriver.run(ConfigurationParser {(int)argv.size()-1, argv.data()});
    });

    compilationErrors = errorStream.str();
    if (compilationErrors.empty()) {
        compiled = true;
    } else {
        std::cerr << compilationErrors;
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

SourceProgram::SourceProgram(std::string sourceCode) : SourceProgram(sourceCode, "test") {}
SourceProgram::SourceProgram(std::string sourceCode, std::string programName) :
    Program{"tmp/" + programName}, programDirectory{getTestResourcePath("programs/tmp/")}
{
    if (mkdir(programDirectory.c_str(), 0777) == -1 && errno != 17) {
        throw std::runtime_error("Could not create directory " + programDirectory + ": " + std::to_string(errno) + ":" + strerror(errno));
    }

    std::ofstream programFile{getSourceFilePath()};
    programFile << sourceCode;
    programFile.close();
}

