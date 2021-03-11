#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"

namespace ast {

class JumpStatement: public AbstractSyntaxTreeNode {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	TerminalSymbol jumpKeyword;
};

} // namespace ast

#endif // _JMP_STMT_NODE_H_
