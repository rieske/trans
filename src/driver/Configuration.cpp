#include "Configuration.h"

#include <stdexcept>

void Configuration::setSourceFiles(std::vector<std::string> sourceFiles) {
    this->sourceFiles = sourceFiles;
}

void Configuration::setResourcesBasePath(std::string resourcesBasePath) {
    this->resourcesBasePath = resourcesBasePath;
}

void Configuration::setGrammarPath(std::string grammarPath) {
    this->grammarPath = grammarPath;
    this->customGrammar = true;
}

void Configuration::setAssemblyDialect(AssemblyDialect dialect) {
    this->assemblyDialect = dialect;
}

void Configuration::enableScannerLogging() {
    this->scannerLogging = true;
}

void Configuration::enableParserLogging() {
    this->parserLogging = true;
}

void Configuration::setCompileOnly(bool compileOnly) {
    this->compileOnly = compileOnly;
}

void Configuration::setOutputPath(std::string outputPath) {
    this->outputPath = std::move(outputPath);
}

void Configuration::setGnuExtensions(bool enabled) {
    gnuExtensions_ = enabled;
}

void Configuration::setPreprocessorStdFlag(std::string stdName) {
    preprocessorStdFlag_ = std::move(stdName);
}

void Configuration::setPreprocessorArgs(std::vector<std::string> args) {
    preprocessorArgs_ = std::move(args);
}

void Configuration::setPreprocessOnly(bool preprocessOnly) {
    preprocessOnly_ = preprocessOnly;
}

std::vector<std::string> Configuration::getSourceFiles() const {
    return sourceFiles;
}

std::string Configuration::getLexPath() const {
    return resourcesBasePath + lexPath;
}

std::string Configuration::getGrammarPath() const {
    return resourcesBasePath + grammarPath;
}

std::string Configuration::getParsingTablePath() const {
    return resourcesBasePath + parsingTablePath;
}

AssemblyDialect Configuration::getAssemblyDialect() const {
    return assemblyDialect;
}

std::string assemblyDialectTag(AssemblyDialect dialect) {
    switch (dialect) {
    case AssemblyDialect::Intel:
        return "intel";
    case AssemblyDialect::AtAndT:
        return "att";
    }
    throw std::logic_error { "unknown AssemblyDialect" };
}

std::string Configuration::assemblyDialectTag() const {
    return ::assemblyDialectTag(assemblyDialect);
}

bool Configuration::usingCustomGrammar() const {
    return customGrammar;
}

bool Configuration::isScannerLoggingEnabled() const {
    return scannerLogging;
}

bool Configuration::isParserLoggingEnabled() const {
    return parserLogging;
}

bool Configuration::isCompileOnly() const {
    return compileOnly;
}

std::string Configuration::getOutputPath() const {
    return outputPath;
}

bool Configuration::gnuExtensions() const {
    return gnuExtensions_;
}

std::string Configuration::getPreprocessorStdFlag() const {
    return preprocessorStdFlag_;
}

const std::vector<std::string>& Configuration::getPreprocessorArgs() const {
    return preprocessorArgs_;
}

bool Configuration::isPreprocessOnly() const {
    return preprocessOnly_;
}
