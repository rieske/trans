#include "add_expr_node.h"

AddExprNode::AddExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
ExprNode(l, children, r, st, ln)
{
    if (reduction == "<add_expr> <add_op> <factor>")
    {
        code = ((ExprNode *)subtrees[0])->getCode();
        value = "rval";
        basic_type = ((ExprNode *)subtrees[0])->getBasicType();
        extended_type = ((ExprNode *)subtrees[0])->getExtendedType();
        SymbolEntry *arg1 = ((ExprNode *)subtrees[0])->getPlace();
        SymbolEntry *arg2 = ((ExprNode *)subtrees[2])->getPlace();
        string check = s_table->typeCheck(arg1, arg2);
        if (check != "ok")
        {
            semanticError(check);
        }
        else
        {
            char op = ((ExprNode *)subtrees[1])->getAttr().at(0);
            vector<Quadruple *> arg2code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), arg2code.begin(), arg2code.end());
            SymbolEntry *res = s_table->newTemp(basic_type, extended_type);
            place = res;
            switch(op)
            {
                case '+':
                    code.push_back(new Quadruple(ADD, arg1, arg2, res));
                    break;
                case '-':
                    code.push_back(new Quadruple(SUB, arg1, arg2, res));
                    break;
                default:
                    semanticError("unidentified add_op operator!\n");
            }
        }
    }
    else if (reduction == "<factor>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
