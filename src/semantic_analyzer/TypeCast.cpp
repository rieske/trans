#include "TypeCast.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string TypeCast::ID { "<cast_expr>" };

TypeCast::TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression) :
		typeSpecifier { typeSpecifier },
		castExpression { std::move(castExpression) } {
	basicType = typeSpecifier.getType();
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
