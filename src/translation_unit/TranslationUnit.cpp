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
    if (lineOffset >= currentLine.length()) {
        if (!advanceLine()) {
            return '\0';
        } else {
            return '\n';
        }
    }
    return currentLine[lineOffset++];
}

bool TranslationUnit::advanceLine() {
    // Skip preprocessor directive lines that can survive host gcc -E -P
    // (e.g. #pragma). Real preprocessing is host gcc; this only keeps the
    // student grammar from seeing leftover # lines.
    while (std::getline(sourceFile, currentLine)) {
        ++currentLineNumber;
        lineOffset = 0;
        std::size_t i = 0;
        while (i < currentLine.size()
                && (currentLine[i] == ' ' || currentLine[i] == '\t')) {
            ++i;
        }
        if (i < currentLine.size() && currentLine[i] == '#') {
            continue;
        }
        return true;
    }
    return false;
}
