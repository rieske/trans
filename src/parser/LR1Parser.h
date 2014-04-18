#ifndef _LR1PARSER_H_
#define _LR1PARSER_H_

#include <stack>
#include <fstream>
#include "Parser.h"
#include "parsing_table.h"
#include "scanner/FiniteAutomatonScanner.h"
#include "semantic_analyzer/syntax_tree.h"
#include "driver/TranslationUnit.h"

using std::stack;
using std::ofstream;

class LR1Parser: public Parser {
public:
	LR1Parser();
	LR1Parser(string gra);
	virtual ~LR1Parser();

	int parse(TranslationUnit& translationUnit);
	SyntaxTree *getSyntaxTree() const;

	static void set_logging(const char *lf);

private:
	void shift(Action *);
	void reduce(Action *);
	void error(Action *);

	void adjustScope();
	void mknode(string left, vector<Node *> children, string reduction);

	void configure_logging();

	void log_syntax_tree() const;

	void fail(string err);

	FiniteAutomatonScanner *scanner;
	Parsing_table *p_table;
	Token *token;
	Token *next_token;
	bool can_forge;
	bool success;
	stack<long> parsing_stack;
	stack<Node *> syntax_stack;

	SyntaxTree *syntax_tree;
	SymbolTable *current_scope;
	vector<ParamDeclNode *> params;

	unsigned line;
	static bool log;
	static ofstream logfile;
	ofstream *output;
	bool custom_grammar;
};

#endif // _LR1PARSER_H_
