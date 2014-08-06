#include "WhileLoopHeader.h"

#include <stddef.h>
#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Expression.h"

namespace semantic_analyzer {

WhileLoopHeader::WhileLoopHeader(std::unique_ptr<Expression> clause, SymbolTable *st) :
		LoopHeader(st),
		clause { std::move(clause) } {
	loop_label = s_table->newLabel();
	code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
	vector<Quadruple *> expr_code = this->clause->getCode();
	code.insert(code.end(), expr_code.begin(), expr_code.end());
	SymbolEntry *expr_val = this->clause->getPlace();
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
	code.push_back(bp);
	backpatchList.push_back(bp);
}

WhileLoopHeader::~WhileLoopHeader() {
}

void WhileLoopHeader::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
