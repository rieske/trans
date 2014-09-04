#ifndef TERMINALSYMBOL_H_
#define TERMINALSYMBOL_H_

#include <string>

#include "../scanner/TranslationUnitContext.h"

namespace ast {

class TerminalSymbol {
public:
	TerminalSymbol(std::string type, std::string value, const TranslationUnitContext& context);
	TerminalSymbol(const TerminalSymbol& that);

	virtual ~TerminalSymbol();

	const std::string type;
	const std::string value;
	const TranslationUnitContext context;
};

} /* namespace ast */

#endif /* TERMINALSYMBOL_H_ */
