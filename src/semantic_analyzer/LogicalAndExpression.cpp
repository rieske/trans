#include "LogicalAndExpression.h"
#include "../code_generator/symbol_table.h"
#include "../code_generator/quadruple.h"

namespace semantic_analyzer {

const std::string LogicalAndExpression::ID { "<log_and_expr>" };

LogicalAndExpression::LogicalAndExpression(LogicalAndExpression* logicalAndExpression, Expression* orExpression, SymbolTable *st,
		unsigned ln) :
		LogicalOrExpression(ID, { logicalAndExpression, orExpression }, st, ln) {
	saveExpressionAttributes(*logicalAndExpression);
	value = "rval";
	backpatchList = logicalAndExpression->getBPList();
	SymbolEntry *arg1 = place;
	SymbolEntry *arg2 = orExpression->getPlace();
	place = s_table->newTemp("int", "");
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = orExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		code.push_back(new Quadruple("0", place));
		code.push_back(new Quadruple(CMP, arg1, 0, NULL));
		Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
		backpatchList.push_back(bp);
		code.push_back(bp);
		code.push_back(new Quadruple(CMP, arg2, 0, NULL));
		bp = new Quadruple(JE, NULL, NULL, NULL);
		backpatchList.push_back(bp);
		code.push_back(bp);
		code.push_back(new Quadruple("1", place));
	}
}

LogicalAndExpression::LogicalAndExpression(Expression* orExpression, SymbolTable *st, unsigned ln) :
		LogicalOrExpression(ID, { orExpression }, st, ln) {
	saveExpressionAttributes(*orExpression);
}

}
