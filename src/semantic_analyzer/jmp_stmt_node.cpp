#include "jmp_stmt_node.h"
#include "expr_node.h"

JmpStmtNode::JmpStmtNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, r, st, ln)
{
    if (reduction == "'continue' ';'")
    {
        cerr << label << "::=" << reduction << " not implemented yet!\n";
    }
    else if (reduction == "'break' ';'")
    {
        cerr << label << "::=" << reduction << " not implemented yet!\n";
    }
    else if (reduction == "'return' <expr> ';'")
    {
        code = ((ExprNode *)subtrees[1])->getCode();
        SymbolEntry *expr_val = ((ExprNode *)subtrees[1])->getPlace();
        code.push_back(new Quadruple(RETURN, expr_val, NULL, NULL));
        attr = expr_val->getBasicType() + expr_val->getExtendedType();
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

string JmpStmtNode::getAttr() const
{
    return attr;
}
