#ifndef PARSER_H_
#define PARSER_H_

class TranslationUnit;
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual int parse(TranslationUnit& translationUnit) = 0;
	virtual SyntaxTree *getSyntaxTree() const = 0;
};

#endif /* PARSER_H_ */
