#include "IOStatement.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string IOStatement::ID { "<io_stmt>" };

IOStatement::IOStatement(TerminalSymbol ioKeyword, std::unique_ptr<Expression> expression) :
		ioKeyword { ioKeyword },
		expression { std::move(expression) } {
	code = this->expression->getCode();
	SymbolEntry *expr_val = this->expression->getPlace();
	if (ioKeyword.value == "output") {
		code.push_back(new Quadruple(OUT, expr_val, NULL, NULL));
	} else if (ioKeyword.value == "input") {
		code.push_back(new Quadruple(IN, expr_val, NULL, NULL));
	} else {
		throw std::runtime_error { "bad IO statement: " + ioKeyword.type };
	}
}

void IOStatement::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
