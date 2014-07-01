#include "log_and_expr_node.h"
#include "../code_generator/symbol_table.h"
#include "../code_generator/quadruple.h"

LogAndExprNode::LogAndExprNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln):
LogExprNode(l, children, production, st, ln)
{
    if (reduction == "<log_and_expr> '&&' <or_expr>")
    {
        getAttributes(0);
        value = "rval";
        backpatchList = ((LogAndExprNode *)subtrees[0])->getBPList();
        SymbolEntry *arg1 = place;
        SymbolEntry *arg2 = ((ExprNode *)subtrees[2])->getPlace();
        place = s_table->newTemp("int", "");
        string check = s_table->typeCheck(arg1, arg2);
        if (check != "ok")
        {
            semanticError(check);
        }
        else
        {
            vector<Quadruple *> arg2code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), arg2code.begin(), arg2code.end());
            code.push_back(new Quadruple("0", place));
            code.push_back(new Quadruple(CMP, arg1, 0, NULL));
            Quadruple *bp = new Quadruple(JE, NULL, NULL, NULL);
            backpatchList.push_back(bp);
            code.push_back(bp);
            code.push_back(new Quadruple(CMP, arg2, 0, NULL));
            bp = new Quadruple(JE, NULL, NULL, NULL);
            backpatchList.push_back(bp);
            code.push_back(bp);
            code.push_back(new Quadruple("1", place));
        }
    }
    else if (reduction == "<or_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
