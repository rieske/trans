#ifndef _POINTER_CAST_NODE_H
#define _POINTER_CAST_NODE_H

#include <memory>

#include "Expression.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

class Pointer;

class PointerCast: public Expression {
public:
	PointerCast(TerminalSymbol typeSpecifier, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression,
			SymbolTable *st);

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

	TerminalSymbol typeSpecifier;
	std::unique_ptr<Pointer> pointer;
	std::unique_ptr<Expression> castExpression;
};

}

#endif // _POINTER_CAST_NODE_H
