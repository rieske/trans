#ifndef PARSER_H_
#define PARSER_H_

class SyntaxTreeBuilder;

class TranslationUnit;
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual int parse(TranslationUnit& translationUnit, SyntaxTreeBuilder& syntaxTreeBuilder) = 0;
	virtual SyntaxTree *getSyntaxTree() const = 0;
};

#endif /* PARSER_H_ */
