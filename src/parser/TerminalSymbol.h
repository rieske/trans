#ifndef TERMINALSYMBOL_H_
#define TERMINALSYMBOL_H_

#include <string>

#include "GrammarSymbol.h"

class TerminalSymbol: public GrammarSymbol {
public:
	TerminalSymbol(const std::string value);
	virtual ~TerminalSymbol();
};

#endif /* TERMINALSYMBOL_H_ */
