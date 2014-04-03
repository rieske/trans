#include "postfix_expr_node.h"
#include "a_expressions_node.h"

PostfixExprNode::PostfixExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
ExprNode(l, children, r, st, ln)
{
    if (reduction == "<postfix_expr> '[' <expr> ']'")
    {
        getAttributes(0);
        if (extended_type.size() && (extended_type.at(0) == 'p' || extended_type.at(0) == 'a'))
        {
            value = "rval";
            extended_type = extended_type.substr(1, extended_type.size());
            SymbolEntry *rval = s_table->newTemp(basic_type, extended_type);
            lval = s_table->newTemp(basic_type, extended_type);
            SymbolEntry *offset = ((ExprNode *)subtrees[2])->getPlace();
            vector<Quadruple *> more_code = ((ExprNode *)subtrees[2])->getCode();
            code.insert(code.end(), more_code.begin(), more_code.end());
            code.push_back(new Quadruple(INDEX, place, offset, rval));
            code.push_back(new Quadruple(INDEX_ADDR, place, offset, lval));
            place = rval;
        }
        else
        {
            printErr();
            cerr << "invalid type for operator[]\n";
        }
    }
    else if (reduction == "<postfix_expr> '(' <a_expressions> ')'")
    {
        place = ((ExprNode *)subtrees[0])->getPlace();
        if ( NULL != place )
        {
            value = "rval";
            basic_type = place->getBasicType();
            extended_type = place->getExtendedType();
            vector<AExprNode *> exprs = ((AExpressionsNode *)subtrees[2])->getExprs();
            if (exprs.size() != place->getParamCount())
            {
                printErr();
                cerr << "no match for function " << basic_type;
                if (extended_type.size())
                {
                    for (unsigned i = 0; i < extended_type.size()-1; i++)
                        cerr << extended_type.at(i);
                }
                cerr << " " << place->getName() << "(";
                if (exprs.size())
                {
                    for (unsigned i = 0; i < exprs.size()-1; i++)
                        cerr << exprs.at(i)->getBasicType() << exprs.at(i)->getExtendedType() << ", ";
                    cerr << exprs.at(exprs.size()-1)->getBasicType() << exprs.at(exprs.size()-1)->getExtendedType();
                }
                    cerr << ")\n";
                return;
            }
            else
            {
                vector<SymbolEntry *> params = place->getParams();
                code = subtrees[2]->getCode();
                for (unsigned i = 0; i < exprs.size(); i++)
                { 
                    SymbolEntry *param = exprs[i]->getPlace();
                    string check;
                    if ( "ok" != (check = s_table->typeCheck(params[i], param)) )
                    {
                        printErr();
                        cerr << check;
                    }
                    code.push_back(new Quadruple(PARAM, param, NULL, NULL));
                }
            }
            code.push_back(new Quadruple(CALL, place, NULL, NULL));
            if (basic_type != "void" || extended_type != "")
            {
                extended_type = extended_type.substr(0, extended_type.size()-1);
                place = s_table->newTemp(basic_type, extended_type);
                code.push_back(new Quadruple(RETRIEVE, place, NULL, NULL));
            }
        }
        else
        {
            printErr();
            cerr << "symbol " << value << " is not defined" << endl;
        }
    }
    else if (reduction == "<postfix_expr> '(' ')'")
    {
        place = ((ExprNode *)subtrees[0])->getPlace();
        if ( NULL != place )
        {
            if (place->getParamCount() == 0)
            {
                value = "rval";
                basic_type = place->getBasicType();
                extended_type = place->getExtendedType();
                code.push_back(new Quadruple(CALL, place, NULL, NULL));
                if (basic_type != "void" || extended_type != "")
                {
                    extended_type = extended_type.substr(0, extended_type.size()-1);
                    place = s_table->newTemp(basic_type, extended_type);
                    code.push_back(new Quadruple(RETRIEVE, place, NULL, NULL));
                }
            }
            else
            {
                printErr();
                cerr << "no match for function " << basic_type;
                cerr << " " << place->getName() << "(";
                cerr << ")\n";
                return;
            }
        }
        else
        {
            printErr();
            cerr << "symbol " << value << " is not defined" << endl;
        }
    }
    else if (reduction == "<postfix_expr> '++'")
    {
        getAttributes(0);
        if (place == NULL || value == "rval")
        {   // dirbama su konstanta
            printErr();
            cerr << "lvalue required as increment operand\n";
        }
        else
        {   // dirbama su kinmamuoju simbolių lentelėje
            code.push_back(new Quadruple(INC, place, NULL, place));
            value = "rval";
        }
    }
    else if (reduction == "<postfix_expr> '--'")
    {
        getAttributes(0);
        if (place == NULL || value == "rval")
        {   // dirbama su konstanta
            printErr();
            cerr << "lvalue required as increment operand\n";
        }
        else
        {   // dirbama su kinmamuoju simbolių lentelėje
            code.push_back(new Quadruple(DEC, place, NULL, place));
            value = "rval";
        }
    }
    else if (reduction == "<term>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}
