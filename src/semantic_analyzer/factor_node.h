#ifndef _FACTOR_NODE_H_
#define _FACTOR_NODE_H_

#include "expr_node.h"

class FactorNode : public ExprNode
{
    public:
        FactorNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _FACTOR_NODE_H_
