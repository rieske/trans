#include "expr_node.h"

ExprNode::ExprNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, r, st, ln)
{
    place = NULL;
    lval = NULL;
    if (label != "expr")
        return;
    if (reduction == "<expr> ',' <a_expr>")
    {
        getAttributes(0);
        vector<Quadruple *> more_code = subtrees[2]->getCode();
        code.insert(code.end(), more_code.begin(), more_code.end());
    }
    else if (reduction == "<a_expr>")
        getAttributes(0);
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &ExprNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel
        << " basic_type=\"" << xmlEncode(basic_type) << "\"";
    if (extended_type != "")
        oss << " extended_type=\"" << extended_type << "\"";
    if (value != "")
        oss << " value=\"" << xmlEncode(value) << "\"";
    else if (place != NULL)
        oss << " place=\"" << place->getName() << "\"";
    oss << " code_size=\"" << code.size() << "\"";
    oss <<" >" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

string ExprNode::getBasicType() const
{
    return basic_type;
}

string ExprNode::getExtendedType() const
{
    return extended_type;
}

string ExprNode::getValue() const
{
    return value;
}

SymbolEntry *ExprNode::getPlace() const
{
    return place;
}

SymbolEntry *ExprNode::getLval() const
{
    return lval;
}

void ExprNode::printCode() const
{
    for (unsigned i = 0; i < code.size(); i++)
        code[i]->output(std::cout);
}

void ExprNode::getAttributes(int subtree)
{
    value = ((ExprNode *)subtrees.at(subtree))->getValue();
    basic_type = ((ExprNode *)subtrees.at(subtree))->getBasicType();
    extended_type = ((ExprNode *)subtrees.at(subtree))->getExtendedType();
    code = ((ExprNode *)subtrees.at(subtree))->getCode();
    place = ((ExprNode *)subtrees.at(subtree))->getPlace();
    lval = ((ExprNode *)subtrees.at(subtree))->getLval();
}

Quadruple *ExprNode::backpatch(vector<Quadruple *> *bpList)
{
    if (bpList->size())
    {
        SymbolEntry *label = s_table->newLabel();
        for (unsigned i = 0; i < bpList->size(); i++)
        {
            bpList->at(i)->setArg1(label);
        }
        bpList->clear();
        return new Quadruple(LABEL, label, NULL, NULL);
    }
    return NULL;
}

Quadruple *ExprNode::backpatch(vector<Quadruple *> *bpList, Quadruple *q)
{
    if (bpList->size())
    {
        SymbolEntry *label = q->getArg1();
        for (unsigned i = 0; i < bpList->size(); i++)
            bpList->at(i)->setArg1(label);
        bpList->clear();
        return q;
    }
    return NULL;
}
