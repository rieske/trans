#include "and_expr_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"

AndExprNode::AndExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
ExprNode(l, children, production, st, ln)
{
    if (reduction == "<and_expr> '&' <eq_expr>")
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
            vector<Quadruple *> arg2code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), arg2code.begin(), arg2code.end());
            place = s_table->newTemp(basic_type, extended_type);
            code.push_back(new Quadruple(AND, arg1, arg2, place));
        }
    }
    else if (reduction == "<eq_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
