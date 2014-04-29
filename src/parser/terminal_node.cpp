#include "terminal_node.h"
#include <iostream>

TerminalNode::TerminalNode(string l, Token *token):
Node(l)
{
    value = token->getLexeme();
}

string TerminalNode::getAttr() const
{
    return value;
}

ostringstream &TerminalNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "<terminal label='" << xmlEncode(label) << "' "
        << "value='" << xmlEncode(value) << "' "
        << " />" << endl;
    return oss;
}
