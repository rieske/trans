#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include "expr_node.h"

class UExprNode : public ExprNode
{
    public:
        UExprNode(string l, vector<Node*> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _U_EXPR_NODE_H_
