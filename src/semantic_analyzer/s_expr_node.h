#ifndef _S_EXPR_NODE_H_
#define _S_EXPR_NODE_H_

#include "expr_node.h"

class SExprNode : public ExprNode
{
    public:
        SExprNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _S_EXPR_NODE_H_
