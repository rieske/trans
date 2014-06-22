#ifndef _POSTFIX_EXPR_NODE_H_
#define _POSTFIX_EXPR_NODE_H_

#include "expr_node.h"

class PostfixExprNode : public ExprNode
{
    public:
        PostfixExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln);

    private:
};

#endif // _POSTFIX_EXPR_NODE_H_
