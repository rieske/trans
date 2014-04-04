#ifndef _FACTOR_NODE_H_
#define _FACTOR_NODE_H_

#include "expr_node.h"

class FactorNode : public ExprNode
{
    public:
        FactorNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _FACTOR_NODE_H_
