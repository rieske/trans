#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "action.h"
#include "item.h"

class Grammar;

using std::string;

class ParsingTable {
public:
	ParsingTable();
	ParsingTable(const string bnfFileName);
	~ParsingTable();

	Action *action(unsigned state, unsigned terminal) const;
	Action *go_to(unsigned state, std::shared_ptr<GrammarSymbol> nonterminal) const;

	int fill_actions(std::vector<std::vector<Item>> C);
	int fill_goto(std::vector<std::vector<Item>> C);
	void fill_errors();

	void print_actions() const;
	void print_goto() const;
	std::shared_ptr<GrammarSymbol> getTerminalById(unsigned id) const;

	void output_html() const;
	void output_table() const;

	void log(std::ostream &out) const;

private:
	void read_table(std::ifstream &table);

	Grammar *grammar;

	unsigned long state_count;
	std::map<std::shared_ptr<GrammarSymbol>, Action *> *action_table;
	std::map<std::shared_ptr<GrammarSymbol>, Action *> *goto_table;
	std::vector<std::shared_ptr<GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<GrammarSymbol>> nonterminals;
	std::map<int, std::shared_ptr<GrammarSymbol>> idToTerminalMappingTable;

	std::vector<std::vector<Item>> items;

	std::map<long, Action *> shifts;
	std::vector<Action *> reductions;
	std::map<long, Action *> gotos;
	std::map<long, Action *> errors;
};

#endif // _PARSING_TABLE_H_
