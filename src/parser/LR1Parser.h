#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "../semantic_analyzer/param_decl_node.h"
#include "Parser.h"

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
	void shift(Action *, TranslationUnit& translationUnit);
	void reduce(Action *);
	void error(Action *, TranslationUnit& translationUnit);

	void adjustScope();
	void mknode(string left, vector<Node *> children, string reduction);

	void configure_logging();

	void log_syntax_tree() const;

	std::unique_ptr<ParsingTable> parsingTable;
	std::unique_ptr<SemanticComponentsFactory> semanticComponentsFactory;

	Token *token;
	Token *next_token;
	bool can_forge;
	bool success;
	stack<long> parsing_stack;
	stack<Node *> syntax_stack;

	SyntaxTree *syntax_tree;
	SymbolTable *current_scope;
	vector<ParamDeclNode *> params;

	unsigned currentLine;
	static bool log;
	static ofstream logfile;
	ofstream *output;
};

#endif // _LR1PARSER_H_
