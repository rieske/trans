#ifndef _PARSING_TABLE_H_
#define _PARSING_TABLE_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "action.h"
#include "LR1Item.h"

class GoTo;
class FirstTable;
class Grammar;

using std::string;

class ParsingTable {
public:
	ParsingTable();
	ParsingTable(const string bnfFileName);
	~ParsingTable();

	const Action& action(unsigned state, unsigned terminal) const;
	const Action& go_to(unsigned state, std::shared_ptr<const GrammarSymbol> nonterminal) const;

	std::shared_ptr<const GrammarSymbol> getTerminalById(unsigned id) const;

	void output_html() const;
	void output_table() const;

	void log(std::ostream &out) const;

private:
	void read_table(std::istream& table);

	int fill_actions(std::vector<std::vector<LR1Item>> C);
	int fill_goto(std::vector<std::vector<LR1Item>> C);
	void fill_errors();

	Grammar *grammar;
	std::unique_ptr<FirstTable> firstTable;
	std::unique_ptr<GoTo> goTo;

	int state_count;
	std::map<std::shared_ptr<const GrammarSymbol>, std::unique_ptr<Action>> *action_table;
	std::map<int, std::map<std::shared_ptr<const GrammarSymbol>, std::unique_ptr<Action>>>goto_table;

	std::map<int, std::shared_ptr<const GrammarSymbol>> idToTerminalMappingTable;

	std::vector<std::vector<LR1Item>> items;
};

#endif // _PARSING_TABLE_H_
