#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include <string>

#include "AbstractSyntaxTreeNode.h"
#include "TerminalSymbol.h"

namespace ast {

class JumpStatement: public AbstractSyntaxTreeNode {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	TerminalSymbol jumpKeyword;
};

}

#endif // _JMP_STMT_NODE_H_
