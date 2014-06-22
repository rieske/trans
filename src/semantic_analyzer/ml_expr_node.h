#ifndef _ML_EXPR_NODE_H_
#define _ML_EXPR_NODE_H_

#include "expr_node.h"

class MLExprNode : public ExprNode
{
    public:
        MLExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _ML_EXPR_NODE_H_
