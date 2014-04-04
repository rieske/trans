#include "u_expr_node.h"

UExprNode::UExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
ExprNode(l, children, r, st, ln)
{
    if (reduction == "'++' <u_expr>")
    {
        getAttributes(1);
        vector<Quadruple *>::iterator it = code.begin();
        if (place == NULL || value == "rval")
        {   // dirbama su konstanta
            printErr();
            cerr << "lvalue required as increment operand\n";
        }
        else
        {
            code.insert(it, new Quadruple(INC, place, NULL, place));
            value = "rval";
        }
    }
    else if (reduction == "'--' <u_expr>")
    {
        getAttributes(1);
        vector<Quadruple *>::iterator it = code.begin();
        if (place == NULL || value == "rval")
        {   // dirbama su konstanta
            printErr();
            cerr << "lvalue required as increment operand\n";
        }
        else
        {
            code.insert(it, new Quadruple(DEC, place, NULL, place));
            value = "rval";
        }
    }
    else if (reduction == "<u_op> <cast_expr>")
    {
        getAttributes(1);
        vector<Quadruple *>::iterator it = code.begin();
        SymbolEntry *temp;
        SymbolEntry *true_label;
        SymbolEntry *exit_label;
        switch (subtrees[0]->getAttr().at(0))
        {
            case '&':
                extended_type = "p" + extended_type;
                temp = s_table->newTemp(basic_type, extended_type);
                code.insert(it, new Quadruple(ADDR, place, NULL, temp));
                place = temp;
                break;
            case '*':
                if (extended_type.size() && extended_type.at(0) == 'p')
                {
                    extended_type = extended_type.substr(1, extended_type.size());
                    temp = s_table->newTemp(basic_type, extended_type);
                    code.insert(code.begin(), new Quadruple(DEREF, place, NULL, temp));
                    place = temp;
                }
                else
                {
                    printErr();
                    cerr << "invalid type argument of ‘unary *’\n";
                }
                break;
            case '+':   // čia kogero nieko ir nereiks
                break;
            case '-':
                value = "rval";
                temp = s_table->newTemp(basic_type, extended_type);
                code.push_back(new Quadruple(UMINUS, place, NULL, temp));
                place = temp;
                break;
            case '!':   // TODO:
                value = "rval";
                temp = s_table->newTemp("int", "");
                true_label = s_table->newLabel();
                exit_label = s_table->newLabel();
                code.push_back(new Quadruple(CMP, place, 0, temp));
                code.push_back(new Quadruple(JE, true_label, NULL, NULL));
                code.push_back(new Quadruple("0", temp));
                code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
                code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
                code.push_back(new Quadruple("1", temp));
                code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
                place = temp;
                break;
            default:
                cerr << "Fatal error! Not recognized unary operator " << subtrees[0]->getAttr().at(0) << endl;
                exit(1);
                break;
        }
    }
    else if (reduction == "<postfix_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
