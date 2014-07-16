#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;

class LoopHeader: public parser::NonterminalNode {
public:
	LoopHeader(Expression* expression, SymbolTable *st, unsigned ln);
	LoopHeader(Expression* initialization, Expression* clause, Expression* increment, SymbolTable *st, unsigned ln);

	vector<Quadruple *> getBPList() const;
	vector<Quadruple *> getAfterLoop() const;
	SymbolEntry *getLoopLabel() const;

	static const std::string ID;

private:
	vector<Quadruple *> backpatchList;
	vector<Quadruple *> afterLoop;
	SymbolEntry *loop_label;
};

}

#endif // _LOOP_HDR_NODE_H_
