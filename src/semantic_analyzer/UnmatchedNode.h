#ifndef _UNMATCHED_NODE_H_
#define _UNMATCHED_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;
class LoopHeader;

class UnmatchedNode: public NonterminalNode {
public:
	UnmatchedNode(ParseTreeNode* ifKeyword, ParseTreeNode* openParenthesis, Expression* expression, ParseTreeNode* closeParenthesis,
			ParseTreeNode* statement, SymbolTable *st, unsigned ln);
	UnmatchedNode(ParseTreeNode* ifKeyword, ParseTreeNode* openParenthesis, Expression* expression, ParseTreeNode* closeParenthesis,
			ParseTreeNode* matched, ParseTreeNode* elseKeyword, ParseTreeNode* unmatched, SymbolTable *st, unsigned ln);
	UnmatchedNode(LoopHeader* loopHeader, ParseTreeNode* unmatched, SymbolTable *st, unsigned ln);

	static const std::string ID;
};

}

#endif // _UNMATCHED_NODE_H_
