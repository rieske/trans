#include "decls_node.h"

DeclsNode::DeclsNode(string l, vector<ParseTreeNode *> &children, Production production):
NonterminalNode(l, children, production)
{
    if (reduction == "<decls> ',' <decl>")
    {
        decls = ((DeclsNode *)subtrees[0])->getDecls();
        decls.push_back((DeclNode*)subtrees[2]);
    }
    else if (reduction == "<decl>")
    {
        decls.push_back((DeclNode*)subtrees[0]);
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &DeclsNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel;
    outputDecls(oss);
    oss << " >" << endl;

    for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

void DeclsNode::outputDecls(ostringstream &oss) const
{
    for (unsigned i = 0; i < decls.size(); i++)
        decls.at(i)->output_attr(oss, i);
}

vector<DeclNode *> DeclsNode::getDecls() const
{
    return decls;
}
