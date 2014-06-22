#include "a_expr_node.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"

AExprNode::AExprNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
LogExprNode(l, children, production, st, ln)
{
    if (reduction == "<u_expr> <a_op> <a_expr>")
    {
        getAttributes(0);
        place = ((ExprNode *)subtrees[0])->getPlace();
        value = ((ExprNode *)subtrees[0])->getValue();
        bool deref = false;
        for (vector<Quadruple *>::iterator it = code.begin(); it != code.end(); it++)
        {
            if ((*it)->getOp() == DEREF)
            {
                deref = true;
            }
        }
        if (value != "lval")
        {
            if (lval == NULL)
            {
                semanticError("lvalue required on the left side of assignment\n");
            }
            place = lval;
        }
        SymbolEntry *arg_place = ((ExprNode *)subtrees[2])->getPlace();
        vector<Quadruple *> a_code = subtrees[2]->getCode();
        code.insert(code.begin(), a_code.begin(), a_code.end());
        string check = s_table->typeCheck(arg_place, place);
        if (check != "ok")
        {
            semanticError(check);
        }
        else
        {
            string op = subtrees[1]->getAttr();
            if (op == "+=")
                code.push_back(new Quadruple(ADD, place, arg_place, place));
            else if (op == "-=")
                code.push_back(new Quadruple(SUB, place, arg_place, place));
            else if (op == "*=")
                code.push_back(new Quadruple(MUL, place, arg_place, place));
            else if (op == "/=")
                code.push_back(new Quadruple(DIV, place, arg_place, place));
            else if (op == "%=")
                code.push_back(new Quadruple(MOD, place, arg_place, place));
            else if (op == "&=")
                code.push_back(new Quadruple(AND, place, arg_place, place));
            else if (op == "^=")
                code.push_back(new Quadruple(XOR, place, arg_place, place));
            else if (op == "|=")
                code.push_back(new Quadruple(OR, place, arg_place, place));
            else if (op == "<<=")
                code.push_back(new Quadruple(SHL, place, arg_place, place));
            else if (op == ">>=")
                code.push_back(new Quadruple(SHR, place, arg_place, place));
            else if (op == "=")
            {
                if (deref)
                {
                    Quadruple *quad = new Quadruple(DEREF_LVAL, arg_place, NULL, code.back()->getArg1());
                    code.pop_back();
                    code.push_back(quad);
                }
                else
                {
                    code.push_back(new Quadruple(ASSIGN, arg_place, NULL, place));
                }
            }
            else
            {
                semanticError("unidentified assignment operator!\n");
                exit(1);
            }
        }
    }
    else if (reduction == "<log_expr>")
    {
        getAttributes(0);
        backpatchList = ((LogExprNode *)subtrees[0])->getBPList();
        Quadruple *q = backpatch(&backpatchList);
        if (q != NULL)
            code.push_back(q);
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
//    printCode();
}

void AExprNode::output_attr(ostringstream &oss, unsigned nr) const
{
    if (basic_type != "")
        oss << " basic_type" << nr << "=\"" << basic_type << "\"";
    if (extended_type != "")
        oss << " extended_type" << nr << "=\"" << extended_type << "\"";
}
