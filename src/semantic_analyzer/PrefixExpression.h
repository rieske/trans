#ifndef PREFIXEXPRESSION_H_
#define PREFIXEXPRESSION_H_

#include <memory>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class PrefixExpression: public Expression {
public:
	PrefixExpression(TerminalSymbol incrementOperator, std::unique_ptr<Expression> unaryExpression, SymbolTable *st);
	virtual ~PrefixExpression();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	TerminalSymbol incrementOperator;
	std::unique_ptr<Expression> unaryExpression;
};

} /* namespace semantic_analyzer */

#endif /* PREFIXEXPRESSION_H_ */
