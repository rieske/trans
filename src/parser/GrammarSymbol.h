#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <stddef.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class GrammarSymbol;

using Production = std::vector<const GrammarSymbol*>;

class GrammarSymbol {
public:
	GrammarSymbol(const std::string name);
	virtual ~GrammarSymbol();

	void addProduction(Production production);

	bool isTerminal() const;
	bool isNonterminal() const;

	std::string getName() const;
	const std::vector<Production>& getProductions() const;

private:
	std::vector<Production> productions;
	const std::string name;
};

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

#endif /* GRAMMARSYMBOL_H_ */
