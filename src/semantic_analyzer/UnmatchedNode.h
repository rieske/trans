#ifndef _UNMATCHED_NODE_H_
#define _UNMATCHED_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;
class LoopHeader;

class UnmatchedNode: public parser::NonterminalNode {
public:
	UnmatchedNode(Expression* expression, parser::ParseTreeNode* statement, SymbolTable *st, unsigned ln);
	UnmatchedNode(Expression* expression, parser::ParseTreeNode* matched, parser::ParseTreeNode* unmatched, SymbolTable *st,
			unsigned ln);
	UnmatchedNode(LoopHeader* loopHeader, parser::ParseTreeNode* unmatched, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _UNMATCHED_NODE_H_
