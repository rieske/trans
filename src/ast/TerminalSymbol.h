#ifndef TERMINALSYMBOL_H_
#define TERMINALSYMBOL_H_

#include <string>

#include "translation_unit/Context.h"

namespace ast {

class TerminalSymbol {
public:
	TerminalSymbol(std::string value, const translation_unit::Context& context);

	std::string value;
	translation_unit::Context context;
};

} // namespace ast

#endif // TERMINALSYMBOL_H_
