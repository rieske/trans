#include "log_expr_node.h"
#include "log_and_expr_node.h"

LogExprNode::LogExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
ExprNode(l, children, r, st, ln)
{
    if (label != "log_expr")
        return;
    if (reduction == "<log_expr> '||' <log_and_expr>")
    {
        getAttributes(0);
        value = "rval";
        SymbolEntry *arg1 = place;
        SymbolEntry *arg2 = ((ExprNode *)subtrees[2])->getPlace();
        place = s_table->newTemp("int", "");
        string check = s_table->typeCheck(arg1, arg2);
        if (check != "ok")
        {
            printErr();
            cerr << check;
        }
        else
        {
            vector<Quadruple *> arg2code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), arg2code.begin(), arg2code.end());
            backpatchList = ((LogAndExprNode *)subtrees[2])->getBPList();
            Quadruple *q = backpatch(&backpatchList);
            if (q != NULL)
                code.push_back(q);
            backpatchList = ((LogAndExprNode *)subtrees[0])->getBPList();
            
            code.push_back(new Quadruple("1", place));
            code.push_back(new Quadruple(CMP, arg1, 0, NULL));
            Quadruple *bp = new Quadruple(JNE, NULL, NULL, NULL);
            backpatchList.push_back(bp);
            code.push_back(bp);
            code.push_back(new Quadruple(CMP, arg2, 0, NULL));
            bp = new Quadruple(JNE, NULL, NULL, NULL);
            backpatchList.push_back(bp);
            code.push_back(bp);
            code.push_back(new Quadruple("0", place));
        }
    }
    else if (reduction == "<log_and_expr>")
    {
        getAttributes(0);
        backpatchList = ((LogAndExprNode *)subtrees[0])->getBPList();
        Quadruple *q = backpatch(&backpatchList);
        if (q != NULL)
            code.push_back(q);
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

vector<Quadruple *> LogExprNode::getBPList() const
{
    return backpatchList;
}
