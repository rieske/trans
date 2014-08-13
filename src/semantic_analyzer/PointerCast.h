#ifndef _POINTER_CAST_NODE_H
#define _POINTER_CAST_NODE_H

#include <memory>

#include "Expression.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

class Pointer;

class PointerCast: public Expression {
public:
	PointerCast(TypeSpecifier typeSpecifier, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression);

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	TypeSpecifier type;
	std::unique_ptr<Pointer> pointer;
	std::unique_ptr<Expression> castExpression;
};

}

#endif // _POINTER_CAST_NODE_H
