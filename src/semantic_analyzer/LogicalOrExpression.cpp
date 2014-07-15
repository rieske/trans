#include "LogicalOrExpression.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "LogicalAndExpression.h"

namespace semantic_analyzer {

const std::string LogicalOrExpression::ID { "<log_expr>" };

LogicalOrExpression::LogicalOrExpression(LogicalOrExpression* logicalOrExpression, LogicalAndExpression* logicalAndExpression,
		SymbolTable *st, unsigned ln) :
		Expression(ID, { logicalOrExpression, logicalAndExpression }, st, ln) {
	getAttributes(0);
	value = "rval";
	SymbolEntry *arg1 = place;
	SymbolEntry *arg2 = logicalAndExpression->getPlace();
	place = s_table->newTemp("int", "");
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = logicalAndExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		backpatchList = logicalAndExpression->getBPList();
		Quadruple *q = backpatch(&backpatchList);
		if (q != NULL)
			code.push_back(q);
		// XXX: this is likely wrong
		backpatchList = logicalOrExpression->getBPList();

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

LogicalOrExpression::LogicalOrExpression(LogicalAndExpression* logicalAndExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { logicalAndExpression }, st, ln) {
	getAttributes(0);
	backpatchList = logicalAndExpression->getBPList();
	Quadruple *q = backpatch(&backpatchList);
	if (q != NULL)
		code.push_back(q);
}

LogicalOrExpression::LogicalOrExpression(std::string label, vector<ParseTreeNode *> children, SymbolTable *st, unsigned ln) :
		Expression(label, children, st, ln) {
}

vector<Quadruple *> LogicalOrExpression::getBPList() const {
	return backpatchList;
}

}
