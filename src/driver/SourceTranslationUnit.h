#ifndef SOURCETRANSLATIONUNIT_H_
#define SOURCETRANSLATIONUNIT_H_

#include <string>

#include "TranslationUnit.h"

class SourceTranslationUnit: public TranslationUnit {
public:
	SourceTranslationUnit(std::string sourceFileName);
	virtual ~SourceTranslationUnit();

	std::string getFileName() const;
	Token getNextToken();

private:
	std::string fileName;
};

#endif /* SOURCETRANSLATIONUNIT_H_ */
