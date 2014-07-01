#ifndef _XOR_EXPR_NODE_H_
#define _XOR_EXPR_NODE_H_

#include "expr_node.h"

class XorExprNode : public ExprNode
{
    public:
        XorExprNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _XOR_EXPR_NODE_H_
