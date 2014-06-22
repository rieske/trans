#include "ptr_node.h"

PtrNode::PtrNode(string l, vector<Node *> &children, Production production):
NonterminalNode(l, children, production)
{
    if (reduction == "<ptr> '*'")
    {
        type = ((PtrNode*)subtrees[0])->getType();
        type = "p" + type;
    }
    else if (reduction == "'*'")
    {
        type = "p";
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &PtrNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel 
        << " type=\""  << type << "\" "
        << ">" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

string PtrNode::getType() const
{
    return type;
}
