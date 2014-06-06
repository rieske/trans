#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <iostream>
#include <string>
#include <fstream>

class Token;

class TranslationUnit {
public:
	TranslationUnit(const std::string sourceFileName);
	virtual ~TranslationUnit();

	virtual std::string getFileName() const;
	virtual int getCurrentLineNumber() const;
	virtual char getNextCharacter();

private:
	bool advanceLine();

	const std::string fileName;
	std::ifstream sourceFile;

	std::string currentLine;
	size_t lineOffset = 0;
	int currentLineNumber = 0;
};

#endif /* TRANSLATIONUNIT_H_ */
