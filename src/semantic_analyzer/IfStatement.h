#ifndef IFSTATEMENT_H_
#define IFSTATEMENT_H_

#include <memory>

#include "NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class IfStatement: public NonterminalNode {
public:
	IfStatement(std::unique_ptr<Expression> testExpression, std::unique_ptr<AbstractSyntaxTreeNode> body, SymbolTable *st);
	virtual ~IfStatement();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<Expression> testExpression;
	const std::unique_ptr<AbstractSyntaxTreeNode> body;
};

} /* namespace semantic_analyzer */

#endif /* IFSTATEMENT_H_ */
