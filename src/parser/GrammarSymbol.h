#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <iostream>
#include <string>
#include <vector>

#include "GrammarRule.h"

class GrammarSymbol {
public:
	GrammarSymbol(const std::string name);
	virtual ~GrammarSymbol();

	bool isTerminal();

	std::string getName() const;
	const std::vector<GrammarRule>& getProductionRules();

protected:
	std::vector<GrammarRule> productionRules;

private:
	const std::string name;

};

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

#endif /* GRAMMARSYMBOL_H_ */
