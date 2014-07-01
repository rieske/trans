#include "matched_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "expr_node.h"
#include "loop_hdr_node.h"

MatchedNode::MatchedNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, production, st, ln)
{
    if (reduction == "<expr> ';'")
    {
        code = ((ExprNode *)subtrees[0])->getCode();
    }
    else if (reduction == "<io_stmt>")
    {
        code = subtrees[0]->getCode();
    }
    else if (reduction == "<jmp_stmt>")
    {
        code = subtrees[0]->getCode();
        attr = subtrees[0]->getAttr();
    }
    else if (reduction == "<block>")
    {
        code = subtrees[0]->getCode();
    }
    else if (reduction == "';'")
    {}
    else if (reduction == "'if' '(' <expr> ')' <matched> 'else' <matched>")
    {   
        code = subtrees[2]->getCode();
        vector<Quadruple *> matched_code = subtrees[4]->getCode();
        vector<Quadruple *> unmatched_code = subtrees[6]->getCode();

        SymbolEntry *else_label = s_table->newLabel();
        SymbolEntry *exit_label = s_table->newLabel();

        matched_code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
        matched_code.push_back(new Quadruple(LABEL, else_label, NULL, NULL));

        unmatched_code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));

        SymbolEntry *expr_val = ((ExprNode *)subtrees[2])->getPlace();
        code.push_back(new Quadruple(CMP, expr_val, 0, NULL));
        code.push_back(new Quadruple(JE, else_label, NULL, NULL));

        code.insert(code.end(), matched_code.begin(), matched_code.end());
        code.insert(code.end(), unmatched_code.begin(), unmatched_code.end());
    }
    else if (reduction == "<loop_hdr> <matched>")
    {
        code = subtrees[0]->getCode();
        vector<Quadruple *> more_code = subtrees[1]->getCode();
        SymbolEntry *loop_label = ((LoopHdrNode *)subtrees[0])->getLoopLabel();
        SymbolEntry *exit_label = s_table->newLabel();

        // backpatch
        vector<Quadruple *> bpList = ((LoopHdrNode *)subtrees[0])->getBPList();
        for (unsigned i = 0; i < bpList.size(); i++)
            bpList[0]->setArg1(exit_label);

        code.insert(code.end(), more_code.begin(), more_code.end());

        vector<Quadruple *> afterLoop = ((LoopHdrNode *)subtrees[0])->getAfterLoop();
        if (afterLoop.size())
        {
            code.insert(code.end(), afterLoop.begin(), afterLoop.end());
        }
        code.push_back(new Quadruple(GOTO, loop_label, NULL, NULL));
        code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

string MatchedNode::getAttr() const
{
    return attr;
}
