#ifndef _MATCHED_NODE_H_
#define _MATCHED_NODE_H_

#include <string>

#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression;
class LoopHeader;

class MatchedNode: public NonterminalNode {
public:
	MatchedNode(Expression* expression, AbstractSyntaxTreeNode* matched, AbstractSyntaxTreeNode* elseMatched, SymbolTable *st, unsigned ln);
	MatchedNode(LoopHeader* loopHeader, AbstractSyntaxTreeNode* matched, SymbolTable *st, unsigned ln);

	string getAttr() const;

	static const std::string ID;
private:
	string attr;
};

}

#endif // _MATCHED_NODE_H_
