#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include "expr_node.h"

class AddExprNode : public ExprNode
{
    public:
        AddExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _ADD_EXPR_NODE_H_
