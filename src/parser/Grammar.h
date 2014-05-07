#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "GrammarSymbol.h"
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
	Rule* getRuleByDefinition(const std::shared_ptr<GrammarSymbol> left, const vector<std::shared_ptr<GrammarSymbol>>& right) const;

	std::set<std::shared_ptr<GrammarSymbol>> getNonterminals() const;
	std::map<int, std::shared_ptr<GrammarSymbol>> getTerminals() const;

	std::shared_ptr<GrammarSymbol> getStartSymbol() const;
	std::shared_ptr<GrammarSymbol> getEndSymbol() const;

	Set_of_items *go_to(Set_of_items *I, std::shared_ptr<GrammarSymbol> X) const;

	std::vector<Set_of_items *> *canonical_collection() const;

	void log(std::ostream &out) const;

private:
	std::shared_ptr<GrammarSymbol> findTerminalByName(std::string& name) const;
	std::shared_ptr<GrammarSymbol> addTerminal(std::string& name);
	std::shared_ptr<GrammarSymbol> addNonterminal(std::string& name);

	Set_of_items *closure(Set_of_items *I) const;

	void readGrammarBnf(std::ifstream& bnfInputStream);
	void computeFirstSets();
	bool addFirst(std::shared_ptr<GrammarSymbol> nonterm, std::shared_ptr<GrammarSymbol> first);
	bool addFirstRow(std::shared_ptr<GrammarSymbol> dest, std::shared_ptr<GrammarSymbol> src);

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

	std::shared_ptr<GrammarSymbol> start_symbol;
	std::shared_ptr<GrammarSymbol> end_symbol;

	std::set<std::shared_ptr<GrammarSymbol>> nonterminals;
	std::set<std::shared_ptr<GrammarSymbol>> terminals;
	std::map<int, std::shared_ptr<GrammarSymbol>> idToTerminalMappingTable;
	std::set<std::shared_ptr<GrammarSymbol>> symbols;

	std::map<std::shared_ptr<GrammarSymbol>, std::set<std::shared_ptr<GrammarSymbol>>> nonterminalFirstSets;
};

#endif // _GRAMMAR_H_
