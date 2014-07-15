#include "UnmatchedNode.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Expression.h"
#include "LoopHeader.h"

namespace semantic_analyzer {

const std::string UnmatchedNode::ID { "<unmatched>" };

UnmatchedNode::UnmatchedNode(Expression* expression, ParseTreeNode* statement, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression, statement }, st, ln) {
	code = expression->getCode();
	vector<Quadruple *> stmt_code = statement->getCode();
	SymbolEntry *exit_label = s_table->newLabel();
	stmt_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
	SymbolEntry *expr_val = expression->getPlace();
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	code.push_back(new Quadruple(JE, exit_label, NULL, NULL));
	code.insert(code.end(), stmt_code.begin(), stmt_code.end());
}

UnmatchedNode::UnmatchedNode(Expression* expression, ParseTreeNode* matched, ParseTreeNode* unmatched, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression, matched, unmatched }, st, ln) {
	code = expression->getCode();
	vector<Quadruple *> matched_code = matched->getCode();
	vector<Quadruple *> unmatched_code = unmatched->getCode();

	SymbolEntry *else_label = s_table->newLabel();
	SymbolEntry *exit_label = s_table->newLabel();

	matched_code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
	matched_code.push_back(new Quadruple(LABEL, else_label, NULL, NULL));

	unmatched_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));

	SymbolEntry *expr_val = ((Expression *) expression)->getPlace();
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	code.push_back(new Quadruple(JE, else_label, NULL, NULL));

	code.insert(code.end(), matched_code.begin(), matched_code.end());
	code.insert(code.end(), unmatched_code.begin(), unmatched_code.end());
}

UnmatchedNode::UnmatchedNode(LoopHeader* loopHeader, ParseTreeNode* unmatched, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { loopHeader, unmatched }, st, ln) {
	code = loopHeader->getCode();
	vector<Quadruple *> more_code = unmatched->getCode();
	SymbolEntry *loop_label = loopHeader->getLoopLabel();
	SymbolEntry *exit_label = s_table->newLabel();

	// backpatch
	vector<Quadruple *> bpList = loopHeader->getBPList();
	for (unsigned i = 0; i < bpList.size(); i++) {
		bpList[0]->setArg1(exit_label);
	}

	code.insert(code.end(), more_code.begin(), more_code.end());

	vector<Quadruple *> afterLoop = loopHeader->getAfterLoop();
	if (afterLoop.size()) {
		code.insert(code.end(), afterLoop.begin(), afterLoop.end());
	}
	code.push_back(new Quadruple(GOTO, loop_label, NULL, NULL));
	code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
}

}
