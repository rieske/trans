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

void Configuration::enableParserLogging() {
    this->parserLogging = true;
}

void Configuration::enableScannerLogging() {
    this->scannerLogging = true;
}

void Configuration::setOutputIntermediateForms() {
    this->outputIntermediateForms = true;
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

bool Configuration::usingCustomGrammar() const {
    return customGrammar;
}

bool Configuration::isParserLoggingEnabled() const {
    return parserLogging;
}

bool Configuration::isScannerLoggingEnabled() const {
    return scannerLogging;
}

bool Configuration::isOutputIntermediateForms() const {
    return outputIntermediateForms;
}
