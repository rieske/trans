#ifndef _LOOP_HDR_NODE_H_
#define _LOOP_HDR_NODE_H_

#include "../parser/nonterminal_node.h"

/**
 * @author Vaidotas Valuckas
 * ciklo antraštės klasė
 **/

class LoopHdrNode : public NonterminalNode
{
    public:
        LoopHdrNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln);

        vector<Quadruple *> getBPList() const;
        vector<Quadruple *> getAfterLoop() const;
        SymbolEntry *getLoopLabel() const;

    private:
        vector<Quadruple *> backpatchList;
        vector<Quadruple *> afterLoop;
        SymbolEntry *loop_label;
};

#endif // _LOOP_HDR_NODE_H_
