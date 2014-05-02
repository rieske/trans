#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

class TranslationUnit;
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual std::unique_ptr<SyntaxTree> parse(TranslationUnit& translationUnit) = 0;
};

#endif /* PARSER_H_ */
