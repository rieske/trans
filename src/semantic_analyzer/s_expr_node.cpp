#include "s_expr_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"

SExprNode::SExprNode(string l, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned ln):
ExprNode(l, children, production, st, ln)
{
    if (reduction == "<s_expr> <s_op> <add_expr>")
    {
        code = ((ExprNode *)subtrees[0])->getCode();
        value = "rval";
        basic_type = ((ExprNode *)subtrees[0])->getBasicType();
        extended_type = ((ExprNode *)subtrees[0])->getExtendedType();
        SymbolEntry *arg1 = ((ExprNode *)subtrees[0])->getPlace();
        SymbolEntry *arg2 = ((ExprNode *)subtrees[2])->getPlace();
        string arg2type = ((ExprNode *)subtrees[2])->getBasicType();
        string arg2etype = ((ExprNode *)subtrees[2])->getExtendedType();
        if (arg2type == "int" && arg2etype == "")
        {
            char op = ((ExprNode *)subtrees[1])->getAttr().at(0);
            vector<Quadruple *> arg2code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), arg2code.begin(), arg2code.end());
            SymbolEntry *res = s_table->newTemp(basic_type, extended_type);
            place = res;
            switch(op)
            {
                case '<':   // <<
                    code.push_back(new Quadruple(SHL, arg1, arg2, res));
                    break;
                case '>':   // >>
                    code.push_back(new Quadruple(SHR, arg1, arg2, res));
                    break;
                default:
                    semanticError("unidentified add_op operator!\n");
            }
        }
        else
        {
            semanticError(" argument of type int required for shift expression\n");
        }
    }
    else if (reduction == "<add_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
