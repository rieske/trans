#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <stddef.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class GrammarSymbol;

using Production = std::vector<std::shared_ptr<GrammarSymbol>>;

class GrammarSymbol {
public:
	GrammarSymbol(const std::string name, const size_t id);
	virtual ~GrammarSymbol();

	bool isTerminal();

	size_t getId() const;
	std::string getName() const;
	const std::vector<Production>& getProductions() const;

protected:
	std::vector<Production> productions;

private:
	size_t id;
	const std::string name;
};

std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& symbol);

#endif /* GRAMMARSYMBOL_H_ */
