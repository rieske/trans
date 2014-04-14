#include "SourceTranslationUnit.h"

#include "../scanner/Token.h"

using std::string;

SourceTranslationUnit::SourceTranslationUnit(string sourceFileName) :
		fileName(sourceFileName) {
}

SourceTranslationUnit::~SourceTranslationUnit() {
	// TODO Auto-generated destructor stub
}

string SourceTranslationUnit::getFileName() const {
	return fileName;
}

Token SourceTranslationUnit::getNextToken() {
	Token tok;
	return tok;
}

