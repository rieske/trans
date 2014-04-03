#ifndef _NONTERMINAL_NODE_H_
#define _NONTERMINAL_NODE_H_

#include "node.h"
#include "../symbol_table.h"

/**
 * @author Vaidotas Valuckas
 * Neterminalinio gramatikos simbolio mazgo sintaksiniame medyje klasė
 **/

class NonterminalNode: public Node
{
    public:
        NonterminalNode(string label, vector<Node *> &children, string r, SymbolTable *st, unsigned ln);
        NonterminalNode(string l, vector<Node *> &children, string r);

        virtual string getAttr() const;
        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
        
        SymbolTable *getScope() const;

    protected:
        string reduction;
        SymbolTable *s_table;
        unsigned line;

    private:
};

#endif // _NONTERMINAL_NODE_H_
