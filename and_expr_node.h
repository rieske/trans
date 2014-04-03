#ifndef _AND_EXPR_NODE_H_
#define _AND_EXPR_NODE_H_

#include "expr_node.h"

class AndExprNode : public ExprNode
{
    public:
        AndExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _AND_EXPR_NODE_H_
