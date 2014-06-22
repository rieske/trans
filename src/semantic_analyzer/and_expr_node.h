#ifndef _AND_EXPR_NODE_H_
#define _AND_EXPR_NODE_H_

#include "expr_node.h"

class AndExprNode : public ExprNode
{
    public:
        AndExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _AND_EXPR_NODE_H_
