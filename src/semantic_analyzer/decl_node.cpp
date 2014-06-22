#include "decl_node.h"
#include "ptr_node.h"

DeclNode::DeclNode(string l, vector<Node *> &children, Production production):
NonterminalNode(l, children, production)
{
    if (reduction == "<ptr> <dir_decl>")
    {   
        type = ((PtrNode*)subtrees[0])->getType();
        name = ((DirDeclNode*)subtrees[1])->getName();
        type = type + ((DirDeclNode*)subtrees[1])->getType();
    }
    else if (reduction == "<dir_decl>")
    {
        name = ((DirDeclNode*)subtrees[0])->getName();
        type = ((DirDeclNode*)subtrees[0])->getType();
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &DeclNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel 
        << " name=\""<< xmlEncode(name) << "\"";
    if (type != "")
        oss << " type=\""<< type << "\"";
    oss <<" >" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

void DeclNode::output_attr(ostringstream &oss, unsigned nr) const
{
    oss << " name" << nr << "=\"" << name << "\"";
    if (type != "")
        oss << " type" << nr << "=\"" << type << "\"";
}

string DeclNode::getName() const
{
    return name;
}

string DeclNode::getType() const
{
    return type;
}
