#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "set_of_items.h"

class Rule;

#define START_SYMBOL "<__start__>"
#define END_SYMBOL "'$end$'"

class Grammar {
public:
	Grammar(const string bnfFileName);
	~Grammar();

	Rule *getRule() const;
	Grammar *getNext() const;

	const std::vector<std::string *> *getNonterminals() const;
	const std::map<unsigned, std::string *> *getTerminals() const;

	Set_of_items *go_to(Set_of_items *I, string *X) const;

	std::vector<Set_of_items *> *canonical_collection() const;

	bool is_terminal(std::string *str) const;
	bool is_nonterminal(std::string *str) const;

	void printAddr() const;

	void log(std::ostream &out) const;

private:
	Grammar(Rule *r);

	Set_of_items *closure(Set_of_items *I) const;

	void addRule(Rule *g);
	void addTerminal(unsigned, std::string *);
	void addNonterminal(std::string *);

	void readGrammarBnf(std::ifstream& bnfInputStream);
	void fillSymbols();
	void fillFirst();
	bool addFirst(std::string *nonterm, std::string *first);
	bool addFirstRow(std::string *dest, std::string *src);

	bool contains(std::vector<std::string *> *vect, std::string *str) const;

	void print_terminals() const;
	void print_nonterminals() const;
	void print_first_table() const;

	void log_terminals(std::ostream &out) const;
	void log_nonterminals(std::ostream &out) const;
	void log_first_table(std::ostream &out) const;

	void print() const;
	void output(std::ostream &out) const;

	// ****************************************************

	Rule *rule;
	Grammar *next;

	std::vector<Rule> rules;

	static std::string *start_symbol;
	static std::string *end_symbol;

	static vector<std::string *> *symbols;
	static vector<std::string *> *nonterminals;
	static std::map<unsigned, std::string *> *terminals;

	static std::map<string *, std::vector<std::string *> *> *first_table;
};

#endif // _GRAMMAR_H_
