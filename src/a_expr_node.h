#ifndef _A_EXPR_NODE_H_
#define _A_EXPR_NODE_H_

#include "log_expr_node.h"

class AExprNode : public LogExprNode
{
    public:
        AExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

        void output_attr(ostringstream &oss, unsigned nr) const;
    private:
};

#endif // _A_EXPR_NODE_H_
