#include "ExpressionList.h"

#include <algorithm>
#include <vector>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ExpressionList::ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide, SymbolTable *st,
		unsigned ln) :
		Expression(st, ln),
		leftHandSide { std::move(leftHandSide) },
		rightHandSide { std::move(rightHandSide) } {
	saveExpressionAttributes(*this->leftHandSide);
	vector<Quadruple *> more_code = this->rightHandSide->getCode();
	code.insert(code.end(), more_code.begin(), more_code.end());
}

ExpressionList::~ExpressionList() {
}

void ExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
