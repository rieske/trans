#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include "../parser/nonterminal_node.h"
#include "decl_node.h"
#include "../code_generator/symbol_table.h"

class VarDeclNode : public NonterminalNode
{
    public:
        VarDeclNode(string l, vector<Node *> &children, string r, SymbolTable *s_t, unsigned ln);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
        
    private:
        string basic_type;
        vector<DeclNode *> decls;
        string init_val;
};

#endif // _VAR_DECL_NODE_H_
