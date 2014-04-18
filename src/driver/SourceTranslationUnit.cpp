#include "SourceTranslationUnit.h"

#include "scanner/Token.h"
#include "scanner/Scanner.h"

#include <stdexcept>

using std::string;

SourceTranslationUnit::SourceTranslationUnit(const string sourceFileName, Scanner& scanner) :
		fileName { sourceFileName },
		sourceFile { sourceFileName },
		scanner(scanner) {
	if (!sourceFile.is_open()) {
		throw std::invalid_argument("Unable to open file " + sourceFileName);
	}
	advanceLine();
}

SourceTranslationUnit::~SourceTranslationUnit() {
	sourceFile.close();
}

string SourceTranslationUnit::getFileName() const {
	return fileName;
}

Token SourceTranslationUnit::getNextToken() {
	return scanner.scan(*this);
}

char SourceTranslationUnit::getNextCharacter() {
	if (lineOffset == currentLine.length()) {
		if (!advanceLine()) {
			return '\0';
		} else {
			return '\n';
		}
	}
	return currentLine[lineOffset++];
}

bool SourceTranslationUnit::advanceLine() {
	if (std::getline(sourceFile, currentLine)) {
		++currentLineNumber;
		lineOffset = 0;
		return true;
	} else {
		return false;
	}
}

unsigned long SourceTranslationUnit::getCurrentLineNumber() const {
	return currentLineNumber;
}
