#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>

#include "../util/Logger.h"
#include "Parser.h"
#include "ParsingTable.h"

class ParsingTable;
class Scanner;
class SemanticComponentsFactory;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(Scanner& scanner) override;

	static void set_logging();

private:
	void log_syntax_tree(SyntaxTree& syntaxTrees) const;

	std::unique_ptr<ParsingTable> parsingTable;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;

	static bool log;
};

#endif // _LR1PARSER_H_
