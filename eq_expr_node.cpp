#include "eq_expr_node.h"

EQExprNode::EQExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
ExprNode(l, children, r, st, ln)
{
    if (reduction == "<eq_expr> <eq_op> <ml_expr>")
    {
        code = ((ExprNode *)subtrees[0])->getCode();
        value = "rval";
        basic_type = "int";
        SymbolEntry *arg1 = ((ExprNode *)subtrees[0])->getPlace();
        SymbolEntry *arg2 = ((ExprNode *)subtrees[2])->getPlace();
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
            code.push_back(new Quadruple(CMP, arg1, arg2, NULL));
            string op = subtrees[1]->getAttr();
            place = s_table->newTemp("int", "");
            SymbolEntry *true_label = s_table->newLabel();
            SymbolEntry *exit_label = s_table->newLabel();
            if (op == "==")
            {
                code.push_back(new Quadruple(JE, true_label, NULL, NULL));
            }
            else if (op == "!=")
            {
                code.push_back(new Quadruple(JNE, true_label, NULL, NULL));
            }
            else
            {
                printErr();
                cerr << "unidentified eq_op operator!\n";
                exit(1);
            }
            code.push_back(new Quadruple("0", place));
            code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
            code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
            code.push_back(new Quadruple("1", place));
            code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
        }
    }
    else if (reduction == "<ml_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
