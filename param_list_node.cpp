#include "param_list_node.h"

ParamListNode::ParamListNode(string left, vector<Node *> &children, string reduction):
NonterminalNode(left, children, reduction)
{
    if (reduction == "<param_list> ',' <param_decl>")
    {
        params = ((ParamListNode *)subtrees[0])->getParams();
        params.push_back((ParamDeclNode *)subtrees[2]);
        if (NULL == params[params.size()-1]->getPlace())
            params.pop_back();
    }
    else if (reduction == "<param_decl>")
    {
        params.push_back((ParamDeclNode *)subtrees[0]);
        if (NULL == params[params.size()-1]->getPlace())
            params.pop_back();
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &ParamListNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel;
    outputParams(oss);
    oss << " >" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

void ParamListNode::outputParams(ostringstream &oss) const
{
    for (unsigned i = 0; i < params.size(); i++)
        params.at(i)->output_attr(oss, i);
}

vector<ParamDeclNode *> ParamListNode::getParams() const
{
    return params;
}
