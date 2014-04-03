#ifndef _IO_STMT_NODE_H_
#define _IO_STMT_NODE_H_

#include "nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * input/output sakinio klasė
 **/

class IOStmtNode : public NonterminalNode
{
    public:
        IOStmtNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

    private:
};

#endif // _IO_STMT_NODE_H_
