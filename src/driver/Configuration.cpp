#include "Configuration.h"

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

void Configuration::enableScannerLogging() {
    this->scannerLogging = true;
}

void Configuration::enableParserLogging() {
    this->parserLogging = true;
}

void Configuration::enableSyntaxTreeLogging() {
    this->syntaxTreeLogging = true;
}

void Configuration::setOutputIntermediateForms() {
    this->outputIntermediateForms = true;
}

void Configuration::addIncludePath(std::string path) {
    includePaths.push_back(std::move(path));
}

void Configuration::addDefine(std::string define) {
    defines.push_back(std::move(define));
}

void Configuration::addUndefine(std::string name) {
    undefines.push_back(std::move(name));
}

void Configuration::setOutputFile(std::string outputFile) {
    this->outputFile = std::move(outputFile);
}

void Configuration::setCompileOnly(bool compileOnly) {
    this->compileOnly = compileOnly;
}

void Configuration::setPreprocessOnly(bool preprocessOnly) {
    this->preprocessOnly = preprocessOnly;
}

void Configuration::setAssemblyOnly(bool assemblyOnly) {
    this->assemblyOnly = assemblyOnly;
}

void Configuration::setSkipPreprocess(bool skip) {
    this->skipPreprocess = skip;
}

void Configuration::setPreprocessor(std::string preprocessor) {
    this->preprocessor = std::move(preprocessor);
}

void Configuration::setObjectFiles(std::vector<std::string> objectFiles) {
    this->objectFiles = std::move(objectFiles);
}

void Configuration::addLinkArg(std::string arg) {
    linkArgs.push_back(std::move(arg));
}

void Configuration::setLinkOnly(bool linkOnly) {
    this->linkOnly = linkOnly;
}

void Configuration::addDepFile(std::string path) {
    depFiles.push_back(std::move(path));
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

const std::string& Configuration::getResourcesBasePath() const {
    return resourcesBasePath;
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

bool Configuration::isSyntaxTreeLoggingEnabled() const {
    return syntaxTreeLogging;
}

bool Configuration::isOutputIntermediateForms() const {
    return outputIntermediateForms;
}

const std::vector<std::string>& Configuration::getIncludePaths() const {
    return includePaths;
}

const std::vector<std::string>& Configuration::getDefines() const {
    return defines;
}

const std::vector<std::string>& Configuration::getUndefines() const {
    return undefines;
}

const std::string& Configuration::getOutputFile() const {
    return outputFile;
}

bool Configuration::isCompileOnly() const {
    return compileOnly;
}

bool Configuration::isPreprocessOnly() const {
    return preprocessOnly;
}

bool Configuration::isAssemblyOnly() const {
    return assemblyOnly;
}

bool Configuration::shouldSkipPreprocess() const {
    return skipPreprocess;
}

const std::string& Configuration::getPreprocessor() const {
    return preprocessor;
}

const std::vector<std::string>& Configuration::getObjectFiles() const {
    return objectFiles;
}

const std::vector<std::string>& Configuration::getLinkArgs() const {
    return linkArgs;
}

bool Configuration::isLinkOnly() const {
    return linkOnly;
}

const std::vector<std::string>& Configuration::getDepFiles() const {
    return depFiles;
}
