#ifndef TERMINALSYMBOL_H_
#define TERMINALSYMBOL_H_

#include <stddef.h>
#include <string>

namespace semantic_analyzer {

class TerminalSymbol {
public:
	TerminalSymbol(std::string type, std::string value, size_t line);
	TerminalSymbol(const TerminalSymbol& that);

	virtual ~TerminalSymbol();

	const std::string type;
	const std::string value;
	const size_t line;
};

} /* namespace semantic_analyzer */

#endif /* TERMINALSYMBOL_H_ */
