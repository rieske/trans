#include "factor_node.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"

FactorNode::FactorNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
ExprNode(l, children, production, st, ln)
{
    if (reduction == "<factor> <m_op> <cast_expr>")
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
                case '*':
                    code.push_back(new Quadruple(MUL, arg1, arg2, res));
                    break;
                case '/':
                    code.push_back(new Quadruple(DIV, arg1, arg2, res));
                    break;
                case '%':
                    code.push_back(new Quadruple(MOD, arg1, arg2, res));
                    break;
                default:
                    semanticError("unidentified m_op operator!\n");
            }
        }
    }
    else if (reduction == "<cast_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
