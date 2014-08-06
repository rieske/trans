#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class AssignmentExpressionList;

class FunctionCall: public Expression {
public:
	FunctionCall(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<AssignmentExpressionList> assignmentExpressionList,
			SymbolTable *st, unsigned ln);
	virtual ~FunctionCall();

	void accept(AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Expression> postfixExpression;
	const std::unique_ptr<AssignmentExpressionList> assignmentExpressionList;
};

} /* namespace semantic_analyzer */

#endif /* FUNCTIONCALL_H_ */
