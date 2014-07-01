#ifndef _MATCHED_NODE_H_
#define _MATCHED_NODE_H_

#include "../parser/nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * input/output sakinio klasė
 **/

class MatchedNode : public NonterminalNode
{
    public:
        MatchedNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln);

        string getAttr() const;

    private:
        string attr;
};

#endif // _MATCHED_NODE_H_
