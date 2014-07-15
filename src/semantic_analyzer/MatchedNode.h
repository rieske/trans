#ifndef _MATCHED_NODE_H_
#define _MATCHED_NODE_H_

#include <string>

#include "../parser/NonterminalNode.h"

namespace semantic_analyzer {

class Expression;
class LoopHeader;

class MatchedNode: public NonterminalNode {
public:
	MatchedNode(Expression* expression, SymbolTable *st, unsigned ln);
	MatchedNode(ParseTreeNode* statement, SymbolTable *st, unsigned ln);
	MatchedNode(ParseTreeNode* ifKeyword, ParseTreeNode* openParenthesis, Expression* expression, ParseTreeNode* closeParenthesis,
			ParseTreeNode* matched, ParseTreeNode* elseKeyword, ParseTreeNode* elseMatched, SymbolTable *st, unsigned ln);
	MatchedNode(LoopHeader* loopHeader, ParseTreeNode* matched, SymbolTable *st, unsigned ln);

	string getAttr() const;

	static const std::string ID;
private:
	string attr;
};

}

#endif // _MATCHED_NODE_H_
