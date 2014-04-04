#ifndef _DIR_DECL_NODE_H_
#define _DIR_DECL_NODE_H_

#include "../parser/nonterminal_node.h"
#include "param_decl_node.h"

class DirDeclNode : public NonterminalNode
{
    public:
        DirDeclNode(string l, vector<Node *> &children, string reduction, SymbolTable *st, unsigned ln);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        string getName() const;
        string getType() const;

        vector<ParamDeclNode *> getParams() const;

    private:
        string name;
        string type;

        vector<ParamDeclNode *> params;
};

#endif // _DIR_DECL_NODE_H_
