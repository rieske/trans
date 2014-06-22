#include "dir_decl_node.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "param_list_node.h"

DirDeclNode::DirDeclNode(string l, vector<Node *> &children, Production production, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, production, st, ln)
{
    if (reduction == "'(' <decl> ')'")  // XXX: čia žiūrėti reiks
    {
    }
    else if (reduction == "'id'")   // pernešam reikšmę ir tiek
    {
        name = subtrees[0]->getAttr();
    }
    else if (reduction == "<dir_decl> '(' <param_list> ')'")    // funkcijos deklaracija
    { 
        name = ((DirDeclNode*)subtrees[0])->getName();
        type = "f";
        params = ((ParamListNode *)subtrees[2])->getParams();
        int errLine;
        if ( 0 != (errLine = s_table->insert(name, "", type, sourceLine)) )
        {
        	std::ostringstream errorDescription;
        	errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line " << errLine << "\n";
            semanticError(errorDescription.str());
        }
        else
        {
            SymbolEntry *place = s_table->lookup(name);
            for (unsigned i = 0; i < params.size(); i++)
            {
                if (params[i]->getPlace() == NULL) 
                    exit(2);
                SymbolEntry *entry = params[i]->getPlace();
                place->setParam(entry);
            }
        }
    }
    else if (reduction == "<dir_decl> '[' <log_expr> ']'")      // masyvo deklaracija
    {
        name = ((DirDeclNode*)subtrees[0])->getName();
        type = "a";
    }
    else if (reduction == "<dir_decl> '(' ')'")     // funkcija be parametrų
    {
        name = ((DirDeclNode*)subtrees[0])->getName();
        type = "f";
        int errLine;
        if ( 0 != (errLine = s_table->insert(name, "", type, sourceLine)) )
        {
        	std::ostringstream errorDescription;
        	errorDescription << "symbol " << name << " declaration conflicts with previous declaration on line "<< errLine << "\n";
            semanticError(errorDescription.str());
        }
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &DirDeclNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel 
        << " name=\""  << xmlEncode(name) << "\" ";
    if (type != "")
        oss << "type=\""  << type << "\" ";
    if (params.size())
        oss << "params=\"true\" ";
    oss << ">" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}

string DirDeclNode::getName() const
{
    return name;
}

string DirDeclNode::getType() const
{
    return type;
}

vector<ParamDeclNode *> DirDeclNode::getParams() const
{
    return params;
}
