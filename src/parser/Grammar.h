#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "rule.h"
#include "set_of_items.h"

class Rule;

#define START_SYMBOL "<__start__>"
#define END_SYMBOL "'$end$'"

class Grammar {
public:
	Grammar(const string bnfFileName);
	~Grammar();

	Rule* getRuleById(int ruleId) const;
	Rule* getRuleByDefinition(const string& left, const vector<string>& right) const;

	const std::vector<std::string> *getNonterminals() const;
	const std::map<unsigned, std::string> *getTerminals() const;

	Set_of_items *go_to(Set_of_items *I, string X) const;

	std::vector<Set_of_items *> *canonical_collection() const;

	bool is_terminal(std::string str) const;
	bool is_nonterminal(std::string str) const;

	void log(std::ostream &out) const;

private:
	Set_of_items *closure(Set_of_items *I) const;

	void addNonterminal(std::string);

	void readGrammarBnf(std::ifstream& bnfInputStream);
	void computeFirstTable();
	bool addFirst(std::string nonterm, std::string first);
	bool addFirstRow(std::string dest, std::string src);

	bool contains(std::vector<std::string>& vect, std::string str) const;

	void print_terminals() const;
	void print_nonterminals() const;
	void print_first_table() const;

	void log_terminals(std::ostream &out) const;
	void log_nonterminals(std::ostream &out) const;
	void log_first_table(std::ostream &out) const;

	void print() const;
	void output(std::ostream &out) const;

	// ****************************************************

	std::vector<Rule*> rules;

	std::string start_symbol;
	std::string end_symbol;

	std::vector<std::string> *nonterminals;
	std::map<unsigned, std::string> *terminals;
	std::set<std::string> symbols;

	std::map<string, std::vector<std::string>> firstTable;
};

#endif // _GRAMMAR_H_
