#include "LoopHeader.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Expression.h"

namespace semantic_analyzer {

const std::string LoopHeader::ID { "<loop_hdr>" };

LoopHeader::LoopHeader(Expression* expression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression }, st, ln) {
	loop_label = s_table->newLabel();
	code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
	vector<Quadruple *> expr_code = expression->getCode();
	code.insert(code.end(), expr_code.begin(), expr_code.end());
	SymbolEntry *expr_val = expression->getPlace();
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
	code.push_back(bp);
	backpatchList.push_back(bp);
}

LoopHeader::LoopHeader(Expression* initialization, Expression* clause, Expression* increment, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { initialization, clause, increment }, st, ln) {
	code = initialization->getCode();    // šitas vieną kartą įvykdomas
	vector<Quadruple *> expr_code = clause->getCode();    // čia tikrinimas pradžioj
	SymbolEntry *expr_val = clause->getPlace();    // tikrinama reikšmė
	afterLoop = increment->getCode();    // šitas po kūno vykdomas

	loop_label = s_table->newLabel();
	code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
	code.insert(code.end(), expr_code.begin(), expr_code.end());
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
	code.push_back(bp);
	backpatchList.push_back(bp);
}

vector<Quadruple *> LoopHeader::getBPList() const {
	return backpatchList;
}

SymbolEntry *LoopHeader::getLoopLabel() const {
	return loop_label;
}

vector<Quadruple *> LoopHeader::getAfterLoop() const {
	return afterLoop;
}

}
