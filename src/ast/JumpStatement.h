#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include "ast/Statement.h"
#include "ast/TerminalSymbol.h"
#include "symbols/AnnotationStore.h"
#include "symbols/LabelEntry.h"

namespace ast {

enum class JumpKind {
    Break,
    Continue
};

class JumpStatement: public Statement {
public:
	JumpStatement(TerminalSymbol jumpKeyword);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	JumpKind jumpKind() const { return kind_; }

	void setJumpTo(symbols::AnnotationStore& store, symbols::LabelEntry label);
	symbols::LabelEntry* getJumpTo(symbols::AnnotationStore& store) const;

	TerminalSymbol jumpKeyword;

private:
	JumpKind kind_;

};

} // namespace ast

#endif // _JMP_STMT_NODE_H_
