#include "a_expressions_node.h"

AExpressionsNode::AExpressionsNode(string l, vector<ParseTreeNode *> &children, Production production):
NonterminalNode(l, children, production)
{
    if (reduction == "<a_expressions> ',' <a_expr>")
    {
        exprs = ((AExpressionsNode *)subtrees[0])->getExprs();
        code = subtrees[0]->getCode();
        exprs.push_back((AExprNode *)subtrees[2]);
        vector<Quadruple *> more_code = subtrees[2]->getCode();
        code.insert(code.end(), more_code.begin(), more_code.end());
    }
    else if (reduction == "<a_expr>")
    {
        exprs.push_back((AExprNode*)subtrees[0]);
        code = subtrees[0]->getCode();
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &AExpressionsNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel;
    outputExprs(oss);
    oss << " code_size=\"" << code.size() << "\"";
    oss << " >" << endl;

    for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

void AExpressionsNode::outputExprs(ostringstream &oss) const
{
    for (unsigned i = 0; i < exprs.size(); i++)
        exprs.at(i)->output_attr(oss, i);
}

vector<AExprNode *> AExpressionsNode::getExprs() const
{
    return exprs;
}
