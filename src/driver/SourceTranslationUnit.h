#ifndef SOURCETRANSLATIONUNIT_H_
#define SOURCETRANSLATIONUNIT_H_

#include <fstream>
#include <string>

#include "TranslationUnit.h"

class Scanner;

class SourceTranslationUnit: public TranslationUnit {
public:
	SourceTranslationUnit(const std::string sourceFileName, Scanner& scanner);
	virtual ~SourceTranslationUnit();

	std::string getFileName() const;
	Token getNextToken();

	char getNextCharacter();
	unsigned long getCurrentLineNumber() const;
private:
	bool advanceLine();

	const std::string fileName;
	std::ifstream sourceFile;

	Scanner& scanner;

	std::string currentLine;
	unsigned long lineOffset = 0;
	unsigned long currentLineNumber = 0;
};

#endif /* SOURCETRANSLATIONUNIT_H_ */
