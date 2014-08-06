#ifndef EXPRESSIONLIST_H_
#define EXPRESSIONLIST_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class ExpressionList: public Expression {
public:
	ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide, SymbolTable *st, unsigned ln);
	virtual ~ExpressionList();

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Expression> leftHandSide;
	const std::unique_ptr<Expression> rightHandSide;
};

} /* namespace semantic_analyzer */

#endif /* EXPRESSIONLIST_H_ */
