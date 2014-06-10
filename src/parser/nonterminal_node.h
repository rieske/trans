#ifndef _NONTERMINAL_NODE_H_
#define _NONTERMINAL_NODE_H_

#include <iostream>
#include <string>
#include <vector>
#include <stdlib.h>
#include "../code_generator/symbol_table.h"


#include "node.h"

class SymbolTable;

/**
 * @author Vaidotas Valuckas
 * Neterminalinio gramatikos simbolio mazgo sintaksiniame medyje klasė
 **/

class NonterminalNode: public Node
{
    public:
        NonterminalNode(string label, vector<Node *> &children, string r, SymbolTable *st, unsigned lineNumber);
        NonterminalNode(string l, vector<Node *> &children, string r);

        virtual string getAttr() const;
        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
        
        SymbolTable *getScope() const;

    protected:

        void semanticError(std::string description);

        string reduction;
        SymbolTable *s_table;
        unsigned sourceLine;

    private:
};

#endif // _NONTERMINAL_NODE_H_
