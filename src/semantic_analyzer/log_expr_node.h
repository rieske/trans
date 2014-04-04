#ifndef _LOG_EXPR_NODE_H_
#define _LOG_EXPR_NODE_H_

#include "expr_node.h"

class LogExprNode : public ExprNode
{
    public:
        LogExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

        vector<Quadruple *> getBPList() const;

    protected:
        vector<Quadruple *> backpatchList;
};

#endif // _LOG_EXPR_NODE_H_
