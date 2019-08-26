#include "TranslationUnit.h"


class Scanner;

using std::string;

TranslationUnit::TranslationUnit(const string sourceFileName) :
        fileName { sourceFileName }, sourceFile { sourceFileName } {
    if (!sourceFile.is_open()) {
        throw std::runtime_error("Unable to open file " + sourceFileName);
    }
    advanceLine();
}

TranslationUnit::~TranslationUnit() {
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
