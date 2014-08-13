#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class AssignmentExpressionList;

class FunctionCall: public Expression {
public:
	FunctionCall(std::unique_ptr<Expression> callExpression, std::unique_ptr<AssignmentExpressionList> argumentList);
	virtual ~FunctionCall();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	const std::unique_ptr<Expression> callExpression;
	const std::unique_ptr<AssignmentExpressionList> argumentList;
};

} /* namespace semantic_analyzer */

#endif /* FUNCTIONCALL_H_ */
