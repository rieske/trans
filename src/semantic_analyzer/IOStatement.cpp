#include "IOStatement.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "Expression.h"
#include "TerminalSymbol.h"


using parser::NonterminalNode;

namespace semantic_analyzer {

const std::string IOStatement::ID { "<io_stmt>" };

IOStatement::IOStatement(TerminalSymbol ioKeyword, Expression* expression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression }, st, ln) {
	code = expression->getCode();
	SymbolEntry *expr_val = expression->getPlace();
	if (ioKeyword.value == "output") {
		code.push_back(new Quadruple(OUT, expr_val, NULL, NULL));
	} else if (ioKeyword.value == "input") {
		code.push_back(new Quadruple(IN, expr_val, NULL, NULL));
	} else {
		semanticError("bad IO statement");
	}
}

}
