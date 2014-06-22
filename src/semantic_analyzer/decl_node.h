#ifndef _DECL_NODE_H_
#define _DECL_NODE_H_

#include "dir_decl_node.h"

class DeclNode : public NonterminalNode
{
    public:
        DeclNode(string l, vector<Node *> &children, Production production);

        virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

        void output_attr(ostringstream &oss, unsigned nr) const;

        string getName() const;
        string getType() const;

    private:
        string name;
        string type;
};

#endif // _DECL_NODE_H_
