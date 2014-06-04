#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>

#include "Parser.h"

class Action;
class ParsingTable;
class SemanticComponentsFactory;
class SyntaxTreeBuilder;
class Token;

using std::stack;
using std::ofstream;

class LR1Parser: public Parser {
public:
	LR1Parser(ParsingTable* parsingTable, SemanticComponentsFactory* semanticComponentsFactory);
	virtual ~LR1Parser();

	std::unique_ptr<SyntaxTree> parse(TranslationUnit& translationUnit);

	static void set_logging(const char *lf);

private:
	void shift(const Action& shiftAction, TranslationUnit& translationUnit, SyntaxTreeBuilder& syntaxTreeBuilder);
	void reduce(const Action& reduceAction, SyntaxTreeBuilder& syntaxTreeBuilder);
	void error(const Action&, TranslationUnit& translationUnit);

	void configure_logging();

	void log_syntax_tree(SyntaxTree& syntaxTrees) const;

	std::unique_ptr<ParsingTable> parsingTable;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;

	Token *token;
	Token *next_token;
	bool can_forge;
	bool success;
	stack<long> parsing_stack;

	static bool log;
	static ofstream logfile;
	ofstream *output;
};

#endif // _LR1PARSER_H_
