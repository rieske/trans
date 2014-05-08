#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "GrammarRule.h"
#include "set_of_items.h"

class Grammar {
public:
	Grammar(const std::vector<std::shared_ptr<GrammarSymbol>> terminals, const std::vector<std::shared_ptr<GrammarSymbol>> nonterminals,
			const std::vector<std::shared_ptr<GrammarRule>> rules);
	~Grammar();

	std::shared_ptr<GrammarRule> getRuleById(int ruleId) const;
	std::shared_ptr<GrammarRule> getRuleByDefinition(const std::shared_ptr<GrammarSymbol> left, const vector<std::shared_ptr<GrammarSymbol>>& right) const;

	std::vector<std::shared_ptr<GrammarSymbol>> getNonterminals() const;
	std::vector<std::shared_ptr<GrammarSymbol>> getTerminals() const;

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

	std::vector<std::shared_ptr<GrammarSymbol>> terminals;
	std::vector<std::shared_ptr<GrammarSymbol>> nonterminals;
	std::vector<std::shared_ptr<GrammarRule>> rules;

	std::shared_ptr<GrammarSymbol> start_symbol;
	std::shared_ptr<GrammarSymbol> end_symbol;

	std::vector<std::shared_ptr<GrammarSymbol>> symbols;

	std::map<std::shared_ptr<GrammarSymbol>, std::vector<std::shared_ptr<GrammarSymbol>>>nonterminalFirstSets;
};

#endif // _GRAMMAR_H_
