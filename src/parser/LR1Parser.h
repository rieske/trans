#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

//#include "../semantic_analyzer/param_decl_node.h"
#include "node.h"
#include "Parser.h"

class SyntaxTreeBuilder;

class Action;
class FiniteAutomatonScanner;
class ParsingTable;
class SemanticComponentsFactory;
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
	void shift(Action *, TranslationUnit& translationUnit, SyntaxTreeBuilder& syntaxTreeBuilder);
	void reduce(Action *, SyntaxTreeBuilder& syntaxTreeBuilder);
	void error(Action *, TranslationUnit& translationUnit);

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
