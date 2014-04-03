#ifndef _CAST_EXPR_NODE_H_
#define _CAST_EXPR_NODE_H_

#include "expr_node.h"

class CastExprNode : public ExprNode
{
    public:
        CastExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _CAST_EXPR_NODE_H_
