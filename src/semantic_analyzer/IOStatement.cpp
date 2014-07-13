#include "IOStatement.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string IOStatement::ID { "<io_stmt>" };

IOStatement::IOStatement(ParseTreeNode* ioKeyword, Expression* expression, ParseTreeNode* semicolon, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { ioKeyword, expression, semicolon }, st, ln) {
	code = expression->getCode();
	SymbolEntry *expr_val = expression->getPlace();
	if (ioKeyword->getAttr() == "output") {
		code.push_back(new Quadruple(OUT, expr_val, NULL, NULL));
	} else if (ioKeyword->getAttr() == "input") {
		code.push_back(new Quadruple(IN, expr_val, NULL, NULL));
	} else {
		semanticError("bad IO statement");
	}
}

}
