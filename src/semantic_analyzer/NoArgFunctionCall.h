#ifndef NOARGFUNCTIONCALL_H_
#define NOARGFUNCTIONCALL_H_

#include <memory>

#include "Expression.h"

namespace semantic_analyzer {

class NoArgFunctionCall: public Expression {
public:
	NoArgFunctionCall(std::unique_ptr<Expression> callExpression, SymbolTable *st, unsigned ln);
	virtual ~NoArgFunctionCall();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	const std::unique_ptr<Expression> callExpression;
};

} /* namespace semantic_analyzer */

#endif /* NOARGFUNCTIONCALL_H_ */
