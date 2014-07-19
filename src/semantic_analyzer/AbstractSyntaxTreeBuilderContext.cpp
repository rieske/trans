#include "AbstractSyntaxTreeBuilderContext.h"

namespace semantic_analyzer {

AbstractSyntaxTreeBuilderContext::AbstractSyntaxTreeBuilderContext() {
}

AbstractSyntaxTreeBuilderContext::~AbstractSyntaxTreeBuilderContext() {
}

void AbstractSyntaxTreeBuilderContext::pushTerminal(TerminalSymbol terminal) {
	terminalSymbols.push(terminal);
}

TerminalSymbol AbstractSyntaxTreeBuilderContext::popTerminal() {
	TerminalSymbol top = terminalSymbols.top();
	terminalSymbols.pop();
	return top;
}

} /* namespace semantic_analyzer */
