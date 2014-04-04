#ifndef _JMP_STMT_NODE_H_
#define _JMP_STMT_NODE_H_

#include "../parser/nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * šuolio sakinio klasė
 **/

class JmpStmtNode : public NonterminalNode
{
    public:
        JmpStmtNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);

        string getAttr() const;

    private:
        string attr;
};

#endif // _JMP_STMT_NODE_H_
