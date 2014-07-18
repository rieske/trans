#include "MatchedNode.h"

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Expression.h"
#include "LoopHeader.h"

namespace semantic_analyzer {

const std::string MatchedNode::ID { "<matched>" };

MatchedNode::MatchedNode(Expression* expression, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression }, st, ln) {
	code = expression->getCode();
}

MatchedNode::MatchedNode(ParseTreeNode* statement, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { statement }, st, ln) {
	code = subtrees[0]->getCode();
	attr = subtrees[0]->getValue();
}

MatchedNode::MatchedNode(Expression* expression, ParseTreeNode* matched, ParseTreeNode* elseMatched, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { expression, matched, elseMatched }, st, ln) {
	code = expression->getCode();
	vector<Quadruple *> matched_code = matched->getCode();
	vector<Quadruple *> unmatched_code = elseMatched->getCode();

	SymbolEntry *else_label = s_table->newLabel();
	SymbolEntry *exit_label = s_table->newLabel();

	matched_code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
	matched_code.push_back(new Quadruple(LABEL, else_label, NULL, NULL));

	unmatched_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));

	SymbolEntry *expr_val = expression->getPlace();
	code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
	code.push_back(new Quadruple(JE, else_label, NULL, NULL));

	code.insert(code.end(), matched_code.begin(), matched_code.end());
	code.insert(code.end(), unmatched_code.begin(), unmatched_code.end());
}

MatchedNode::MatchedNode(LoopHeader* loopHeader, ParseTreeNode* matched, SymbolTable *st, unsigned ln) :
		NonterminalNode(ID, { loopHeader, matched }, st, ln) {
	code = loopHeader->getCode();
	vector<Quadruple *> more_code = matched->getCode();
	SymbolEntry *loop_label = loopHeader->getLoopLabel();
	SymbolEntry *exit_label = s_table->newLabel();

	// backpatch
	vector<Quadruple *> bpList = loopHeader->getBPList();
	for (unsigned i = 0; i < bpList.size(); i++)
		bpList[0]->setArg1(exit_label);

	code.insert(code.end(), more_code.begin(), more_code.end());

	vector<Quadruple *> afterLoop = loopHeader->getAfterLoop();
	if (afterLoop.size()) {
		code.insert(code.end(), afterLoop.begin(), afterLoop.end());
	}
	code.push_back(new Quadruple(GOTO, loop_label, NULL, NULL));
	code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
}

string MatchedNode::getAttr() const {
	return attr;
}

}
