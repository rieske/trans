#ifndef NONTERMINAMSYMBOL_H_
#define NONTERMINAMSYMBOL_H_

#include <string>

#include "GrammarSymbol.h"

class NonterminamSymbol: public GrammarSymbol {
public:
	NonterminamSymbol(const std::string value);
	virtual ~NonterminamSymbol();

	bool isTerminal() const;
};

#endif /* NONTERMINAMSYMBOL_H_ */
