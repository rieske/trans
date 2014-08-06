#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include <string>

#include "NonterminalNode.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class JumpStatement: public NonterminalNode {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	TerminalSymbol jumpKeyword;
};

}

#endif // _JMP_STMT_NODE_H_
