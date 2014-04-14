#ifndef TRANSLATIONUNIT_H_
#define TRANSLATIONUNIT_H_

#include <string>

class Token;

class TranslationUnit {
public:
	virtual ~TranslationUnit() {
	}

	virtual std::string getFileName() const = 0;
	virtual Token getNextToken() = 0;
};

#endif /* TRANSLATIONUNIT_H_ */
