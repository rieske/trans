#ifndef RETURNSTATEMENT_H_
#define RETURNSTATEMENT_H_

#include <memory>

#include "AbstractSyntaxTreeNode.h"

namespace semantic_analyzer {

class Expression;

class ReturnStatement: public AbstractSyntaxTreeNode {
public:
	ReturnStatement(std::unique_ptr<Expression> returnExpression);
	virtual ~ReturnStatement();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	std::unique_ptr<Expression> returnExpression;
};

} /* namespace semantic_analyzer */

#endif /* RETURNSTATEMENT_H_ */
