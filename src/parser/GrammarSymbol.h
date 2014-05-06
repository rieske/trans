#ifndef GRAMMARSYMBOL_H_
#define GRAMMARSYMBOL_H_

#include <string>

class GrammarSymbol {
public:
	GrammarSymbol(const std::string value);
	virtual ~GrammarSymbol();

	bool operator<(const GrammarSymbol& rhs) const;

	virtual bool isTerminal() const = 0;
private:
	const std::string value;
};

#endif /* GRAMMARSYMBOL_H_ */
