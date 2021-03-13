#include "TranslationUnit.h"

TranslationUnit::TranslationUnit(const std::string sourceFileName) :
        fileName { sourceFileName }, sourceFile { sourceFileName } {
    if (!sourceFile.is_open()) {
        throw std::runtime_error("Unable to open file " + sourceFileName);
    }
    advanceLine();
}

translation_unit::Context TranslationUnit::getContext() const {
    return {fileName, currentLineNumber};
}

char TranslationUnit::getNextCharacter() {
    if (lineOffset == currentLine.length()) {
        if (!advanceLine()) {
            return '\0';
        } else {
            return '\n';
        }
    }
    return currentLine[lineOffset++];
}

bool TranslationUnit::advanceLine() {
    if (std::getline(sourceFile, currentLine)) {
        ++currentLineNumber;
        lineOffset = 0;
        return true;
    } else {
        return false;
    }
}

