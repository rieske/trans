#ifndef _IO_STMT_NODE_H_
#define _IO_STMT_NODE_H_

#include <memory>
#include <string>

#include "NonterminalNode.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class Expression;

class IOStatement: public NonterminalNode {
public:
	IOStatement(TerminalSymbol ioKeyword, std::unique_ptr<Expression> expression);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	TerminalSymbol ioKeyword;
	std::unique_ptr<Expression> expression;
};

}

#endif // _IO_STMT_NODE_H_
