#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

class GrammarSymbol;

using Production = std::vector<std::shared_ptr<GrammarSymbol>>;

class GrammarSymbol {
public:
	GrammarSymbol(const std::string name);
	virtual ~GrammarSymbol();

	bool isTerminal();

	std::string getName() const;
	const std::vector<Production>& getProductions();

protected:
	std::vector<Production> productions;

private:
	const std::string name;

};

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

#endif /* GRAMMARSYMBOL_H_ */
