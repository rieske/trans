#ifndef SCANNER_H_
#define SCANNER_H_

#include "Token.h"

class TranslationUnit;

class Scanner {
public:
	virtual ~Scanner() {
	}

	virtual Token scan(TranslationUnit& translationUnit) = 0;
};

#endif /* SCANNER_H_ */
