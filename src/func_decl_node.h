#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include "parser/nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * Funkcijos deklaracijos klasė
 **/

class FuncDeclNode : public NonterminalNode
{
    public:
        FuncDeclNode(string l, vector<Node *> &children, string r, SymbolTable *s_t, unsigned ln);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

    private:
        string name;
        string basic_type;
        string extended_type;
};

#endif // _FUNC_DECL_NODE_H_
