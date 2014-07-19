#ifndef _IO_STMT_NODE_H_
#define _IO_STMT_NODE_H_

#include <string>

#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression;
class TerminalSymbol;

class IOStatement: public NonterminalNode {
public:
	IOStatement(TerminalSymbol ioKeyword, Expression* expression, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _IO_STMT_NODE_H_
