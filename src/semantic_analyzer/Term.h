#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include <string>

#include "Expression.h"

namespace semantic_analyzer {

class TerminalSymbol;

class Term: public Expression {
public:
	Term(TerminalSymbol term, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _TERM_NODE_H_
