#ifndef NONTERMINAMSYMBOL_H_
#define NONTERMINAMSYMBOL_H_

#include <string>

#include "GrammarSymbol.h"

class NonterminalSymbol: public GrammarSymbol {
public:
	NonterminalSymbol(const std::string value);
	virtual ~NonterminalSymbol();

	void addProduction(Production production);
};

#endif /* NONTERMINAMSYMBOL_H_ */
