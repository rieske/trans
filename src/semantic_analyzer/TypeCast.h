#ifndef TYPECAST_H_
#define TYPECAST_H_

#include <memory>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class TypeCast: public Expression {
public:
	TypeCast(TerminalSymbol typeSpecifier, std::unique_ptr<Expression> castExpression, SymbolTable *st);
	virtual ~TypeCast();

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	static const std::string ID;

	TerminalSymbol typeSpecifier;
	std::unique_ptr<Expression> castExpression;
};

} /* namespace semantic_analyzer */

#endif /* TYPECAST_H_ */
