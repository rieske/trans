#ifndef _LOG_AND_EXPR_NODE_H_
#define _LOG_AND_EXPR_NODE_H_

#include "log_expr_node.h"

class LogAndExprNode : public LogExprNode
{
    public:
        LogAndExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln);

};

#endif // _LOG_AND_EXPR_NODE_H_
