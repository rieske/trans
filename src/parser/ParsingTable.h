#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "action.h"
#include "set_of_items.h"

class Grammar;

class ParsingTable {
public:
	ParsingTable();
	ParsingTable(const string bnfFileName);
	~ParsingTable();

	Action *action(unsigned state, unsigned terminal) const;
	Action *go_to(unsigned state, std::string nonterminal) const;

	int fill_actions(std::vector<Set_of_items *> *C);
	int fill_goto(std::vector<Set_of_items *> *C);
	void fill_errors();

	void print_actions() const;
	void print_goto() const;
	std::string getTerminalById(unsigned id) const;

	void output_html() const;
	void output_table() const;

	void log(std::ostream &out) const;

private:
	bool v_are_equal(std::vector<string> v1, std::vector<string> v2) const;

	void read_table(std::ifstream &table);

	Grammar *grammar;

	unsigned long state_count;
	std::map<string, Action *> *action_table;
	std::map<string, Action *> *goto_table;
	std::vector<std::string> nonterminals;
	std::map<unsigned, std::string> terminals;

	std::vector<Set_of_items *> *items;

	std::map<long, Action *> shifts;
	std::vector<Action *> reductions;
	std::map<long, Action *> gotos;
	std::map<long, Action *> errors;
};

#endif // _PARSING_TABLE_H_
