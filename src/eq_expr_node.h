#ifndef _EQ_EXPR_NODE_H_
#define _EQ_EXPR_NODE_H_

#include "expr_node.h"

class EQExprNode : public ExprNode
{
    public:
        EQExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _EQ_EXPR_NODE_H_
