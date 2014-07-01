#include "param_decl_node.h"
#include "decl_node.h"

ParamDeclNode::ParamDeclNode(string left, vector<ParseTreeNode*> &children, Production production, SymbolTable *st, unsigned ln):
NonterminalNode(left, children, production, st, ln)
{
    if (reduction == "<type_spec> <decl>")
    {
        basic_type = subtrees[0]->getAttr();
        extended_type = ((DeclNode *)subtrees[1])->getType();
        name = ((DeclNode *)subtrees[1])->getName();
        if (basic_type == "void" && extended_type == "")
        {
            semanticError("error: function parameter ‘" + name + "’ declared void\n");
        }
        else
        {
            place = new SymbolEntry(name, basic_type, extended_type, false, sourceLine);
            place->setParam();
        }
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

string ParamDeclNode::getBasicType() const
{
    return basic_type;
}

string ParamDeclNode::getExtendedType() const
{
    return extended_type;
}

ostringstream &ParamDeclNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel
        << " basic_type=\"" << xmlEncode(basic_type) << "\"";
    if (extended_type != "")
        oss << " extended_type=\"" << xmlEncode(extended_type) << "\"";
    ((DeclNode *)subtrees[1])->output_attr(oss, 0);
    oss <<" >" << endl;

    for (vector<ParseTreeNode *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

void ParamDeclNode::output_attr(ostringstream &oss, unsigned nr) const
{
    oss << " name" << nr << "=\"" << name << "\"";
    if (basic_type != "")
        oss << " basic_type" << nr << "=\"" << basic_type << "\"";
    if (extended_type != "")
        oss << " extended_type" << nr << "=\"" << extended_type << "\"";
}

SymbolEntry *ParamDeclNode::getPlace() const
{
    return place;
}
