#include "LogicalOrExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string LogicalOrExpression::ID { "<log_expr>" };

LogicalOrExpression::LogicalOrExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide,
		SymbolTable *st, unsigned ln) :
		Expression(st, ln),
		leftHandSide { std::move(leftHandSide) },
		rightHandSide { std::move(rightHandSide) } {
	saveExpressionAttributes (*this->leftHandSide);
	value = "rval";
	SymbolEntry *arg1 = place;
	SymbolEntry *arg2 = this->rightHandSide->getPlace();
	place = s_table->newTemp(BasicType::INTEGER, "");
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = this->rightHandSide->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		backpatchList = this->rightHandSide->getBackpatchList();
		backpatch();

		backpatchList = this->leftHandSide->getBackpatchList();
		code.push_back(new Quadruple("1", place));
		code.push_back(new Quadruple(CMP, arg1, 0, NULL));
		Quadruple *bp = new Quadruple(JNE, NULL, NULL, NULL);
		backpatchList.push_back(bp);
		code.push_back(bp);
		code.push_back(new Quadruple(CMP, arg2, 0, NULL));
		bp = new Quadruple(JNE, NULL, NULL, NULL);
		backpatchList.push_back(bp);
		code.push_back(bp);
		code.push_back(new Quadruple("0", place));
	}
}

void LogicalOrExpression::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
