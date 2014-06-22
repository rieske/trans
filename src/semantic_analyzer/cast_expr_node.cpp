#include "cast_expr_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "ptr_node.h"

CastExprNode::CastExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
ExprNode(l, children, production, st, ln)
{
    if (reduction == "'(' <type_spec> ')' <cast_expr>")
    {   
        basic_type = subtrees[1]->getAttr();
        extended_type = "";
        SymbolEntry *arg = ((ExprNode *)subtrees[4])->getPlace();
        place = s_table->newTemp(basic_type, extended_type);
        code = subtrees[3]->getCode();
        code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
        value = "rval";
    }
    else if (reduction == "'(' <type_spec> <ptr> ')' <cast_expr>")
    {
        basic_type = subtrees[1]->getAttr();
        extended_type = ((PtrNode *)subtrees[2])->getType();
        SymbolEntry *arg = ((ExprNode *)subtrees[4])->getPlace();
        place = s_table->newTemp(basic_type, extended_type);
        code = subtrees[4]->getCode();
        code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
        value = "rval";
    }
    else if (reduction == "<u_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
