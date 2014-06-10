#include "var_decl_node.h"
#include "carrier_node.h"
#include "decls_node.h"
#include "expr_node.h"

VarDeclNode::VarDeclNode(string l, vector<Node *> &children, string r, SymbolTable *st, unsigned ln):
NonterminalNode(l, children, r, st, ln)
{
    if (reduction == "<type_spec> <decls> ';'")
    {
        basic_type = subtrees[0]->getAttr();
        decls = ((DeclsNode *)subtrees[1])->getDecls();
        int errLine;
        for (vector<DeclNode *>::const_iterator it = decls.begin(); it != decls.end(); it++)
            if (basic_type == "void" && (*it)->getType() == "")
            {
                semanticError("error: variable or field ‘" + (*it)->getName() + "’ declared void\n");
            }
            else if ( 0 != (errLine = s_table->insert((*it)->getName(), basic_type, (*it)->getType(), line)) )
            {
            	std::ostringstream errorDescription;
            	errorDescription <<"symbol " << (*it)->getName() << " declaration conflicts with previous declaration on line " << errLine << "\n";
                semanticError(errorDescription.str());
            }
    }
    else if (reduction == "<type_spec> <decls> '=' <a_expr> ';'")
    {
        basic_type = subtrees[0]->getAttr();
        decls = ((DeclsNode *)subtrees[1])->getDecls();
        int errLine;
        for (vector<DeclNode *>::const_iterator it = decls.begin(); it != decls.end(); it++)
            if (basic_type == "void" && (*it)->getType() == "")
            {
                semanticError("error: variable or field ‘" + (*it)->getName() + "’ declared void\n");
                return;
            }
            else if ( 0 != (errLine = s_table->insert((*it)->getName(), basic_type, (*it)->getType(), line)) )
            {
            	std::ostringstream errorDescription;
            	errorDescription << "symbol " << (*it)->getName() << " declaration conflicts with previous declaration on line " << errLine << "\n";
                semanticError(errorDescription.str());
                return;
            }
        vector<Quadruple *> a_code = subtrees[3]->getCode();
        code.insert(code.end(), a_code.begin(), a_code.end());
        code.push_back(new Quadruple( ASSIGN, ((ExprNode *)subtrees[3])->getPlace(),
                        NULL, s_table->lookup(decls[decls.size()-1]->getName() )));        
    }
    else
    {
        cerr << "Error! No semantic actions found for rule { " << label << " ::= " << reduction << " }!\n";
        exit(1);
    }
}

ostringstream &VarDeclNode::asXml(ostringstream &oss, unsigned depth) const
{
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    string elabel = xmlEncode(label);
    oss << "<" << elabel
        << " basic_type=\"" << xmlEncode(basic_type) << "\"";
    for (unsigned i = 0; i < decls.size(); i++)
        decls.at(i)->output_attr(oss, i);
    if (init_val != "")
        oss << " init_val=\""<< init_val << "\"";
    oss <<" >" << endl;

    for (vector<Node *>::const_iterator it = subtrees.begin(); it != subtrees.end(); it++)
        (*it)->asXml(oss, depth+1);
    
    for (unsigned i = 0; i < depth; i++)
        oss << "    ";
    oss << "</" << elabel << ">" << endl;
    return oss;
}
