#include "loop_hdr_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "expr_node.h"

LoopHdrNode::LoopHdrNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, production, st, ln)
{
    if (reduction == "'while' '(' <expr> ')'")
    {
        loop_label = s_table->newLabel();
        code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
        vector<Quadruple *> expr_code = ((ExprNode *)subtrees[2])->getCode();
        code.insert(code.end(), expr_code.begin(), expr_code.end());
        SymbolEntry *expr_val = ((ExprNode *)subtrees[2])->getPlace();
        code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
        Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
        code.push_back(bp);
        backpatchList.push_back(bp);
    }
    else if (reduction == "'for' '(' <expr> ';' <expr> ';' <expr> ')'")
    {
        code = ((ExprNode *)subtrees[2])->getCode();    // šitas vieną kartą įvykdomas
        vector<Quadruple *> expr_code = ((ExprNode *)subtrees[4])->getCode();  // čia tikrinimas pradžioj
        SymbolEntry *expr_val = ((ExprNode *)subtrees[4])->getPlace(); // tikrinama reikšmė
        afterLoop = ((ExprNode *)subtrees[6])->getCode();  // šitas po kūno vykdomas

        loop_label = s_table->newLabel();
        code.push_back(new Quadruple(LABEL, loop_label, NULL, NULL));
        code.insert(code.end(), expr_code.begin(), expr_code.end());
        code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
        Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
        code.push_back(bp);
        backpatchList.push_back(bp);
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

vector<Quadruple *> LoopHdrNode::getBPList() const
{
    return backpatchList;
}

SymbolEntry *LoopHdrNode::getLoopLabel() const
{
    return loop_label;
}

vector<Quadruple *> LoopHdrNode::getAfterLoop() const
{
    return afterLoop;
}
