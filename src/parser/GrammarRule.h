#ifndef _RULE_H_
#define _RULE_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class GrammarSymbol;

using Production = std::vector<std::shared_ptr<GrammarSymbol>>;

class GrammarRule {
public:
	GrammarRule(const std::shared_ptr<GrammarSymbol> nonterminal, const std::vector<std::shared_ptr<GrammarSymbol>> production,
			const int ruleId);
	~GrammarRule();

	std::shared_ptr<GrammarSymbol> getNonterminal() const;
	Production getProduction() const;

	std::string rightStr() const;

	int getId() const;

private:
	int id;
	std::shared_ptr<GrammarSymbol> nonterminal;
	Production production;

	friend std::ostream& operator<<(std::ostream& out, const GrammarRule& rule);
};

#endif // _RULE_H_
