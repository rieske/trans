#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/TerminalSymbol.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

class JumpStatement: public AbstractSyntaxTreeNode {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	void setJumpTo(symbols::AnnotationStore& store, symbols::LabelEntry label);
	symbols::LabelEntry* getJumpTo(symbols::AnnotationStore& store) const;

	TerminalSymbol jumpKeyword;

};

} // namespace ast

#endif // _JMP_STMT_NODE_H_
