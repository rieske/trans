#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>

#include "../driver/TranslationUnit.h"
#include "../util/Logger.h"
#include "Parser.h"

class Action;
class ParsingTable;
class Scanner;
class SemanticComponentsFactory;
class SyntaxTreeBuilder;
class Token;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory, Logger logger);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(Scanner& scanner) override;

	static void set_logging();

private:
	void shift(const Action& shiftAction, Scanner& scanner, SyntaxTreeBuilder& syntaxTreeBuilder);
	void reduce(const Action& reduceAction, SyntaxTreeBuilder& syntaxTreeBuilder);
	void error(const Action&, Scanner& scanner);

	void log_syntax_tree(SyntaxTree& syntaxTrees) const;

	std::unique_ptr<ParsingTable> parsingTable;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;

	Token *token;
	Token *next_token;
	bool can_forge;
	bool success;
	std::stack<long> parsing_stack;

	static bool log;

	Logger logger;
};

#endif // _LR1PARSER_H_
