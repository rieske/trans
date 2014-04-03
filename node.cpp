#include "syntax_tree.h"
#include "node.h"

Node::Node(string l)
{
    error = false;
    assign_label(l);
}

Node::Node(string l, vector<Node *> &children)
{
    error = false;
    assign_label(l);
    assign_children(children);
}

Node::~Node()
{
    for (vector<Node *>::iterator it = subtrees.begin(); it != subtrees.end(); it++)
        delete *it;
}

void Node::assign_label(string &l)
{
    label = "";
    if (l.size() > 0)
    {
        if (l.at(l.size()-1) == '>')
            l = l.substr(0, l.size()-1);
        if (l.at(0) == '<')
            l = l.substr(1, l.size()-1);

        if (l.at(l.size()-1) == '\'')
            l = l.substr(0, l.size()-1);
        if (l.at(0) == '\'')
            l = l.substr(1, l.size()-1);
        
        label = l; 
    }
    else
        label = "undefined";
}

void Node::assign_children(vector<Node *> &children)
{
    for (int i = children.size()-1; i >= 0; i--)
        subtrees.push_back(children.at(i));
}

string Node::xmlEncode(const string &str) const
{
    string ret;
    for (unsigned i = 0; i < str.size(); i++)
    {
        switch (str.at(i))
        {
            case '&':
                ret += "&amp;";
                continue;
            case '<':
                ret += "&lt;";
                continue;
            case '>':
                ret += "&gt;";
                continue;
            case '"':
            case '\'':
                ret += "&quot;";
                continue;
            default:
                ret += str.at(i);
        }
    }
    return ret;
}

bool Node::getErrorFlag() const
{
    return error;
}

void Node::printErr()
{
    error = true;
    cerr << SyntaxTree::getFileName() << ":" << SyntaxTree::getLine() << ": error: ";
}

vector<Quadruple *> Node::getCode() const
{
    return code;
}
