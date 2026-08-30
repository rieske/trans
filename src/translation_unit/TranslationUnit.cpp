#include "TranslationUnit.h"

#include <cctype>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string_view>

namespace {

bool parseLineMarker(std::string_view line, std::size_t hashIndex,
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
        sourceName.assign(line.data() + begin, i - begin);
    }
    return true;
}

} // namespace

TranslationUnit::TranslationUnit(const std::string sourceFileName) :
        context_ { sourceFileName, 0 } {
    std::ifstream sourceFile { sourceFileName };
    if (!sourceFile.is_open()) {
        throw std::runtime_error("Unable to open file " + sourceFileName);
    }
    source.assign(std::istreambuf_iterator<char> { sourceFile }, std::istreambuf_iterator<char> {});
    advanceLine();
}

char TranslationUnit::getNextCharacter() {
    if (lineOffset >= currentLine.size()) {
        if (!advanceLine()) {
            return '\0';
        }
        return '\n';
    }
    return currentLine[lineOffset++];
}

bool TranslationUnit::advanceLine() {
    while (readPos < source.size()) {
        const std::size_t lineStart = readPos;
        const std::size_t nl = source.find('\n', readPos);
        if (nl == std::string::npos) {
            currentLine = std::string_view { source }.substr(lineStart);
            readPos = source.size();
        } else {
            currentLine = std::string_view { source }.substr(lineStart, nl - lineStart);
            readPos = nl + 1;
        }
        std::string name { context_.getSourceName() };
        std::size_t lineNumber = context_.getOffset() + 1;
        lineOffset = 0;
        std::size_t i = 0;
        while (i < currentLine.size()
                && (currentLine[i] == ' ' || currentLine[i] == '\t')) {
            ++i;
        }
        const bool hashLine = i < currentLine.size() && currentLine[i] == '#';
        if (hashLine) {
            std::size_t markerLine = 0;
            std::string markerName;
            if (parseLineMarker(currentLine, i, markerLine, markerName)) {
                if (!markerName.empty()) {
                    name = std::move(markerName);
                }
                lineNumber = markerLine == 0 ? 0 : markerLine - 1;
            }
        }
        context_ = translation_unit::Context { std::move(name), lineNumber };
        if (!hashLine) {
            return true;
        }
    }
    currentLine = {};
    lineOffset = 0;
    return false;
}

