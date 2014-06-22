#include "func_decl_node.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "decl_node.h"

FuncDeclNode::FuncDeclNode(string l, vector<Node *> &children, Production production, SymbolTable *s_t, unsigned ln):
NonterminalNode(l, children, production, s_t, ln)
{
    if (reduction == "<type_spec> <decl> <block>")
    {
        name = ((DeclNode *)subtrees[1])->getName();
        basic_type = subtrees[0]->getAttr();
        extended_type = ((DeclNode *)subtrees[1])->getType();
        code = subtrees[2]->getCode();
        SymbolEntry *entry = s_table->lookup(name);
        if (entry != NULL)
        {
            if ( "" == entry->getBasicType() || basic_type == entry->getBasicType() )
            {
                entry->setBasicType(basic_type);
                entry->setExtendedType(extended_type);
            }
            else
            {
                semanticError("recursive function " + name + " call used with wrong type! Can't convert "
                        + entry->getBasicType() + " to " + basic_type + "\n");
            }
            code.insert(code.begin(), new Quadruple(PROC, entry, NULL, NULL));
            code.insert(code.end(), new Quadruple(ENDPROC, entry, NULL, NULL));
        }
        else
        {
            semanticError("fatal error: could't lookup function entry to fill return values!\n");
            exit(1);
        }
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &FuncDeclNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel
        << " basic_type=\"" << xmlEncode(basic_type) << "\"";
    if (extended_type != "")
        oss << " extended_type=\"" << xmlEncode(extended_type) << "\"";
    oss <<" >" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}
