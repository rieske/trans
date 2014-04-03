#include "nonterminal_node.h"

NonterminalNode::NonterminalNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
Node(l, children),
reduction(r),
s_table(st),
line(ln)
{}

NonterminalNode::NonterminalNode(string l, vector<Node *> &children, string r):
Node(l, children),
reduction(r),
s_table(NULL),
line(0)
{}

string NonterminalNode::getAttr() const
{
    return "";
}

ostringstream &NonterminalNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel << ">" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

SymbolTable *NonterminalNode::getScope() const
{
    return s_table;
}
