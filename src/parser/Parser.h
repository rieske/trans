#ifndef PARSER_H_
#define PARSER_H_
class SyntaxTree;

class Parser {
public:
	virtual ~Parser() {
	}

	virtual int parse(const char *src) = 0;
	virtual SyntaxTree *getSyntaxTree() const = 0;
};

#endif /* PARSER_H_ */
