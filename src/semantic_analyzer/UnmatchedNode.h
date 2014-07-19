#ifndef _UNMATCHED_NODE_H_
#define _UNMATCHED_NODE_H_

#include <string>

#include "NonterminalNode.h"

class SymbolTable;

namespace semantic_analyzer {

class Expression;
class LoopHeader;

class UnmatchedNode: public NonterminalNode {
public:
	UnmatchedNode(Expression* expression, AbstractSyntaxTreeNode* statement, SymbolTable *st, unsigned ln);
	UnmatchedNode(Expression* expression, AbstractSyntaxTreeNode* matched, AbstractSyntaxTreeNode* unmatched, SymbolTable *st,
			unsigned ln);
	UnmatchedNode(LoopHeader* loopHeader, AbstractSyntaxTreeNode* unmatched, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _UNMATCHED_NODE_H_
