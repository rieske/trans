#include "Configuration.h"

#include <stdexcept>

void Configuration::setSourceFiles(std::vector<std::string> sourceFiles) {
    this->sourceFiles = sourceFiles;
}

void Configuration::setResourcesBasePath(std::string resourcesBasePath) {
    this->resourcesBasePath = std::move(resourcesBasePath);
}

bool Configuration::hasResourcesBasePath() const {
    return !resourcesBasePath.empty();
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

namespace {

// Explicit restrictiveness rank: higher stops earlier. Independent of enum storage order.
constexpr int stopAfterRank(StopAfter stage) {
    switch (stage) {
    case StopAfter::Link:
        return 0;
    case StopAfter::Object:
        return 1;
    case StopAfter::Assembly:
        return 2;
    case StopAfter::Preprocess:
        return 3;
    }
    return 0;
}

} // namespace

void Configuration::setStopAfter(StopAfter stage) {
    if (stopAfterRank(stage) > stopAfterRank(stopAfter_)) {
        stopAfter_ = stage;
    }
}

void Configuration::setCompileOnly() {
    setStopAfter(StopAfter::Object);
}

void Configuration::setAssemblyOnly() {
    setStopAfter(StopAfter::Assembly);
}

void Configuration::setSaveTemps(bool saveTemps) {
    saveTemps_ = saveTemps;
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

void Configuration::setLinkerArgs(std::vector<std::string> args) {
    linkerArgs_ = std::move(args);
}

void Configuration::setPreprocessOnly() {
    setStopAfter(StopAfter::Preprocess);
}

void Configuration::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void Configuration::setIgnoredFlags(std::vector<std::string> flags) {
    ignoredFlags_ = std::move(flags);
}

std::vector<std::string> Configuration::getSourceFiles() const {
    return sourceFiles;
}

std::string Configuration::getLexPath() const {
    return resourcesBasePath + lexPath;
}

std::string Configuration::getGrammarPath() const {
    if (customGrammar) {
        return grammarPath;
    }
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

StopAfter Configuration::stopAfter() const {
    return stopAfter_;
}

bool Configuration::stopsBeforeLink() const {
    return stopAfter_ != StopAfter::Link;
}

bool Configuration::isCompileOnly() const {
    return stopAfter_ == StopAfter::Object;
}

bool Configuration::isAssemblyOnly() const {
    return stopAfter_ == StopAfter::Assembly;
}

bool Configuration::isSaveTemps() const {
    return saveTemps_;
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

const std::vector<std::string>& Configuration::getLinkerArgs() const {
    return linkerArgs_;
}

bool Configuration::isPreprocessOnly() const {
    return stopAfter_ == StopAfter::Preprocess;
}

bool Configuration::isVerbose() const {
    return verbose_;
}

const std::vector<std::string>& Configuration::getIgnoredFlags() const {
    return ignoredFlags_;
}
