#ifndef ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_
#define ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_

#include <stack>

#include "TerminalSymbol.h"

namespace semantic_analyzer {

class AbstractSyntaxTreeBuilderContext {
public:
	AbstractSyntaxTreeBuilderContext();
	virtual ~AbstractSyntaxTreeBuilderContext();

	void pushTerminal(TerminalSymbol terminal);
	TerminalSymbol popTerminal();

private:
	std::stack<TerminalSymbol> terminalSymbols;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEBUILDERCONTEXT_H_ */
