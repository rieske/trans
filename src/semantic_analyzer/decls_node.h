#ifndef _DECLS_NODE_H_
#define _DECLS_NODE_H_

#include "../parser/nonterminal_node.h"
#include "decl_node.h"

class DeclsNode : public NonterminalNode
{
    public:
        DeclsNode(string l, vector<Node *> &children, Production production);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        vector<DeclNode *> getDecls() const;

        void outputDecls(ostringstream &oss) const;

    private:
        vector<DeclNode *> decls;
};

#endif // _DECLS_NODE_H_
