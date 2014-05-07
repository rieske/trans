#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <iostream>
#include <string>

class GrammarSymbol {
public:
	GrammarSymbol(const std::string name);
	virtual ~GrammarSymbol();

	virtual bool isTerminal() const = 0;

	std::string getName() const;
private:
	const std::string name;

	friend std::ostream& operator<<(std::ostream& ostream, const GrammarSymbol& state);
};

#endif /* GRAMMARSYMBOL_H_ */
