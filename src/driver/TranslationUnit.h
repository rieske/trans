#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <string>

class Token;

class TranslationUnit {
public:
	TranslationUnit() {
	}
	TranslationUnit(const TranslationUnit& that) = delete;
	virtual ~TranslationUnit() {
	}

	virtual std::string getFileName() const = 0;
	virtual Token getNextToken() = 0;
	virtual char getNextCharacter() = 0;
};

#endif /* TRANSLATIONUNIT_H_ */
