#include "nonterminal_node.h"

#include <iterator>

#include "../semantic_analyzer/SyntaxTree.h"

NonterminalNode::NonterminalNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned lineNumber):
Node(l, children),
reduction(r),
s_table(st),
sourceLine(lineNumber)
{}

NonterminalNode::NonterminalNode(string l, vector<Node *> &children, string r):
NonterminalNode(l, children, r , nullptr, 0)
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

void NonterminalNode::semanticError(std::string description)
{
    error = true;
    cerr << SyntaxTree::getFileName() << ":" << sourceLine << ": error: " << description;
}
