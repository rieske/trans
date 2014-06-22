#ifndef _OR_EXPR_NODE_H_
#define _OR_EXPR_NODE_H_

#include "expr_node.h"

class OrExprNode : public ExprNode
{
    public:
        OrExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _OR_EXPR_NODE_H_
