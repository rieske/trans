#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>

#include "Parser.h"

class ParsingTable;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(Scanner& scanner, SemanticAnalyzer& semanticAnalyzer) override;

	static void set_logging();

private:
	void log_syntax_tree(SyntaxTree& syntaxTrees) const;

	std::unique_ptr<ParsingTable> parsingTable;

	static bool log;
};

#endif // _LR1PARSER_H_
