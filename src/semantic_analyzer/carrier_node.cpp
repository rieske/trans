#include "carrier_node.h"

CarrierNode::CarrierNode(string l, vector<ParseTreeNode *> &children):
NonterminalNode(l, children, {})
{
    if (children.size() == 1)
    {
        attr = children.at(0)->getAttr();
        vector<Quadruple *> c_code = children[0]->getCode();
        code.insert(code.end(), c_code.begin(), c_code.end());
    }
    else
    {
        for (unsigned i = 0; i < children.size(); i++)
        {
            vector<Quadruple *> c_code = children[i]->getCode();
            code.insert(code.begin(), c_code.begin(), c_code.end());
        }
    }
}

CarrierNode::CarrierNode(string l, vector<ParseTreeNode *> &children, SymbolTable *scope):
NonterminalNode(l, children, {})
{
    s_table = scope;
    if (children.size() == 1)
    {
        attr = children.at(0)->getAttr();
        vector<Quadruple *> c_code = children[0]->getCode();
        code.insert(code.end(), c_code.begin(), c_code.end());
    }
    else
    {
        for (unsigned i = 0; i < children.size(); i++)
        {
            vector<Quadruple *> c_code = children[i]->getCode();
            code.insert(code.begin(), c_code.begin(), c_code.end());
        }
    }
}

string CarrierNode::getAttr() const
{
    return attr;
}

ostringstream &CarrierNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel << " attribute=\""<< xmlEncode(attr) << "\""
        << " code_size=\"" << code.size() << "\" "
        << ">" << endl;

    for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

