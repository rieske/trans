#include "TranslationUnit.h"

#include <cctype>

namespace {

bool parseLineMarker(const std::string& line, std::size_t hashIndex,
        std::size_t& lineNumber, std::string& sourceName) {
    std::size_t i = hashIndex + 1;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
        ++i;
    }
    if (i + 4 <= line.size()
            && line.compare(i, 4, "line") == 0
            && (i + 4 == line.size() || line[i + 4] == ' ' || line[i + 4] == '\t')) {
        i += 4;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
            ++i;
        }
    }
    if (i >= line.size() || !std::isdigit(static_cast<unsigned char>(line[i]))) {
        return false;
    }
    std::size_t number = 0;
    while (i < line.size() && std::isdigit(static_cast<unsigned char>(line[i]))) {
        number = number * 10 + static_cast<std::size_t>(line[i] - '0');
        ++i;
    }
    lineNumber = number;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) {
        ++i;
    }
    if (i < line.size() && line[i] == '"') {
        ++i;
        const std::size_t begin = i;
        while (i < line.size() && line[i] != '"') {
            ++i;
        }
        sourceName = line.substr(begin, i - begin);
    }
    return true;
}

} // namespace

TranslationUnit::TranslationUnit(const std::string sourceFileName) :
        currentSourceName { sourceFileName }, sourceFile { sourceFileName } {
    if (!sourceFile.is_open()) {
        throw std::runtime_error("Unable to open file " + sourceFileName);
    }
    advanceLine();
}

translation_unit::Context TranslationUnit::getContext() const {
    return {currentSourceName, currentLineNumber};
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
    // Leftover preprocessor lines after host gcc -E (-P still emits #pragma).
    while (std::getline(sourceFile, currentLine)) {
        ++currentLineNumber;
        lineOffset = 0;
        std::size_t i = 0;
        while (i < currentLine.size()
                && (currentLine[i] == ' ' || currentLine[i] == '\t')) {
            ++i;
        }
        if (i < currentLine.size() && currentLine[i] == '#') {
            std::size_t markerLine = 0;
            std::string markerName;
            if (parseLineMarker(currentLine, i, markerLine, markerName)) {
                currentLineNumber = markerLine == 0 ? 0 : markerLine - 1;
                if (!markerName.empty()) {
                    currentSourceName = markerName;
                }
            }
            continue;
        }
        return true;
    }
    currentLine.clear();
    lineOffset = 0;
    return false;
}

