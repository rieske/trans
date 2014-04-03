#ifndef _TERM_NODE_H_
#define _TERM_NODE_H_

#include "expr_node.h"

class TermNode : public ExprNode
{
    public:
        TermNode(string l, vector<Node *> &children, string reduction, SymbolTable *st, unsigned ln);

    private:
};

#endif // _TERM_NODE_H_
