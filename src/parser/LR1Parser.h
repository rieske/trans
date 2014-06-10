#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>

#include "../semantic_analyzer/SemanticAnalyzer.h"
#include "../util/Logger.h"
#include "Parser.h"
#include "ParsingTable.h"

class Action;
class LR1Item;
class ParsingTable;
class Scanner;
class SemanticComponentsFactory;
class Token;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory, Logger logger);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(Scanner& scanner) override;

	static void set_logging();

private:
	void shift(const parse_state state, Token& currentToken, Scanner& scanner, SemanticAnalyzer& semanticAnalyzer);
	void reduce(const LR1Item& reduction, Token& currentToken, SemanticAnalyzer& semanticAnalyzer);
	void error(const Action&, Token& currentToken, Scanner& scanner, SemanticAnalyzer& semanticAnalyzer);

	void log_syntax_tree(SyntaxTree& syntaxTrees) const;

	std::unique_ptr<ParsingTable> parsingTable;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;

	bool currentTokenIsForged;
	std::stack<parse_state> parsing_stack;

	static bool log;

	Logger logger;
};

#endif // _LR1PARSER_H_
