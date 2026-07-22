#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include <memory>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "semantic_analyzer/LabelEntry.h"

namespace ast {

class JumpStatement: public AbstractSyntaxTreeNode {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	void setJumpTo(semantic_analyzer::LabelEntry label);
	semantic_analyzer::LabelEntry* getJumpTo() const;

	TerminalSymbol jumpKeyword;

private:
	std::unique_ptr<semantic_analyzer::LabelEntry> jumpTo;
};

} // namespace ast

#endif // _JMP_STMT_NODE_H_
