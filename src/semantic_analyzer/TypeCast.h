#ifndef TYPECAST_H_
#define TYPECAST_H_

#include <memory>
#include <string>

#include "Expression.h"
#include "TypeSpecifier.h"

namespace semantic_analyzer {

class TypeCast: public Expression {
public:
	TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression, SymbolTable *st);
	virtual ~TypeCast();

	void accept(AbstractSyntaxTreeVisitor& visitor) override;

	static const std::string ID;

	TypeSpecifier typeSpecifier;
	std::unique_ptr<Expression> castExpression;
};

} /* namespace semantic_analyzer */

#endif /* TYPECAST_H_ */
