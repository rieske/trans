#ifndef TERMINALSYMBOL_H_
#define TERMINALSYMBOL_H_

#include <string>

#include "translation_unit/Context.h"

namespace ast {

class TerminalSymbol {
public:
	TerminalSymbol(std::string type, std::string value, const translation_unit::Context& context);
	TerminalSymbol(const TerminalSymbol& that);

	const std::string type;
	const std::string value;
	const translation_unit::Context context;
};

} /* namespace ast */

#endif /* TERMINALSYMBOL_H_ */
